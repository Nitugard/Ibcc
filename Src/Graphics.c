/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Graphics.h"

#ifndef CORE_ASSERT
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#else
#include <GL/gl3w.h>
#endif

#include "Allocator.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./ThirdParty/stbi_image.h"

typedef void(*gfx_shader_recompile_callback)(gfx_shader_handle handle);

typedef struct gfx_shader_uniform_command{
    char name[MAXIMUM_UNIFORM_NAME_LENGTH];
    gfx_type type;
    int32_t * reference_index;
}gfx_pipeline_uniform_command;

typedef struct gfx_shader_attribute{
    int32_t id; //attribute id
    bool enabled; //whether attribute is enabled in shader program
    enum gfx_type type; //attribute type
    int32_t length; //in units of the type
    char name[MAXIMUM_ATTRIBUTE_NAME_LENGTH]; //attribute name
} gfx_shader_attribute;

typedef struct gfx_shader_uniform{
    int32_t id; //uniform id
    bool enabled; //whether uniform is enabled in shader program
    enum gfx_type type; //uniform type
    int32_t size; //in units of the bytes
    int32_t length; //in units of the type
    char name[MAXIMUM_UNIFORM_NAME_LENGTH]; //uniform name
    void (*uniform_setter)(uint32_t id, void* buffer); //uniform setter, cached
} gfx_shader_uniform;

typedef struct gfx_shader{
    int32_t id; //program id
    enum gfx_resource_status status; //resource creation status
    char name[MAXIMUM_SHADER_NAME_LENGTH]; //name of the shader, provided by user
    struct gfx_shader_attribute attributes[MAXIMUM_ATTRIBUTES_PER_SHADER]; //active attributes
    struct gfx_shader_uniform uniforms[MAXIMUM_UNIFORMS_PER_SHADER]; //active uniforms
    int32_t attr_count; //how many attributes are in the list
    int32_t uniform_count; //how many uniforms are in the list
    int32_t vs, fs; //references to the shaders during creation process
    char* vs_data;
    char* fs_data; //shader stage raw ascii data
    gfx_pipeline_handle pipelines[MAXIMUM_PIPELINES_PER_SHADER]; //pipelines that depend on this shader
    int32_t pipeline_count;
    struct gfx_shader_uniform_command uniform_commands[MAXIMUM_UNIFORM_COMMANDS_PER_SHADER]; //active shader uniforms
    int32_t uniform_commands_count;
} gfx_shader;


typedef struct gfx_buffer{
    uint32_t id; //opengl index
    enum gfx_resource_status status;
    enum gfx_buffer_type type;
    enum gfx_buffer_update_mode mode;
} gfx_buffer;

typedef struct gfx_pipeline_attr_command{
    char name[MAXIMUM_ATTRIBUTE_NAME_LENGTH];
    gfx_buffer_handle buffer;
    int32_t count;
    int32_t offset;
    int32_t stride;
} gfx_pipeline_attr_command;

typedef struct gfx_pipeline_ebo_command{
    gfx_buffer_handle buffer;
    bool enabled;
} gfx_pipeline_ebo_command;


typedef struct gfx_pipeline{
    uint32_t vao_id;
    uint32_t ebo_id;
    enum gfx_resource_status status;
    gfx_shader_handle shader_handle;
    int32_t shader_pipeline_index;
    struct gfx_pipeline_attr_command attr_commands[MAXIMUM_ATTRIBUTE_COMMANDS_PER_PIPELINE];
    int32_t attrs_commands_count;
    gfx_pipeline_ebo_command ebo_command;
    bool disable_depth_buffer;
}gfx_pipeline;

typedef struct gfx_texture{
    uint32_t id;
    int32_t width;
    int32_t height;
    gfx_texture_filter_mode filter;
    gfx_texture_wrap_mode wrap;
    gfx_texture_type type;
    enum gfx_resource_status status;
} gfx_texture;

typedef struct gfx_texture_cubemap{
    gfx_texture texture;
} gfx_texture_cubemap;

typedef struct gfx_framebuffer{
    uint32_t id;
    int32_t width;
    int32_t height;
    gfx_texture color;
    gfx_texture depth;
    gfx_texture_filter_mode filter;
    enum gfx_resource_status status;
} gfx_framebuffer;

typedef struct gfx_draw_pass{
    enum gfx_pass_action action;
    enum gfx_pass_option option;
    float color[4];
    int32_t draw_calls;
    gfx_framebuffer_handle fbo;
} gfx_draw_pass;

gfx_draw_pass draw_pass_array[MAXIMUM_DRAW_PASSES_PER_FRAME];
int32_t draw_pass_count;


gfx_log_callback gfx_log;
shader_change_callback shader_change;

char log_buffer[LOG_BUFFER_SIZE];


#define SHADER_CHANGE_CALLBACK(shader) if(shader_change != 0) shader_change(shader);
#define LOG_ERROR(format, ...) if(gfx_log != 0) { sprintf(log_buffer, format, __VA_ARGS__); gfx_log(log_buffer, true);}
#define LOG(format, ...)  if(gfx_log != 0) { sprintf(log_buffer, format, __VA_ARGS__); gfx_log(log_buffer, false);}
#define LOGG(format)  if(gfx_log != 0) gfx_log(format, true);
#define LOGG_ERROR(format) if(gfx_log != 0) gfx_log(format, true);

/*
 * Uniform settters.
 */

void uniform_set_f1(uint32_t id, void* buffer){glUniform1f(id, *(float*)buffer);}
void uniform_set_f2(uint32_t id, void* buffer){glUniform2f(id, *(float*)buffer, *((float*)buffer + 1));}
void uniform_set_f3(uint32_t id, void* buffer){glUniform3f(id, *(float*)buffer, *((float*)buffer + 1), *((float*)buffer + 2));}
void uniform_set_f4(uint32_t id, void* buffer){glUniform4f(id, *(float*)buffer, *((float*)buffer + 1), *((float*)buffer + 2), *((float*)buffer + 3));}
void uniform_set_i1(uint32_t id, void* buffer){glUniform1i(id, *(int*)buffer);}
void uniform_set_i2(uint32_t id, void* buffer){glUniform2i(id, *(int*)buffer, *((int*)buffer + 1));}
void uniform_set_i3(uint32_t id, void* buffer){glUniform3i(id, *(int*)buffer, *((int*)buffer + 1), *((int*)buffer + 2));}
void uniform_set_i4(uint32_t id, void* buffer){glUniform4i(id, *(int*)buffer, *((int*)buffer + 1), *((int*)buffer + 2), *((int*)buffer + 3));}
void uniform_set_m2(uint32_t id, void* buffer){glUniformMatrix2fv(id, 1, 1, (float*)buffer);}
void uniform_set_m3(uint32_t id, void* buffer){glUniformMatrix3fv(id, 1, 1, (float*)buffer);}
void uniform_set_m4(uint32_t id, void* buffer){glUniformMatrix4fv(id, 1, 1, (float*)(buffer));}
void uniform_set_sampler(uint32_t id, void* buffer){ glBindSampler(id, *(int32_t*)buffer); };
void uniform_set_texture(uint32_t id, void* buffer){ glBindTexture(id, *(int32_t*)buffer); };

bool gfx_init() {

#ifdef __EMSCRIPTEN__
#else
    gl3wInit();
#endif

    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_CLAMP);
    glDepthFunc(GL_LEQUAL);

    draw_pass_count = 0;

    shader_change = 0;
    gfx_log = 0;

    return true;
}

void gfx_terminate() {
}

void gfx_log_callback_set(gfx_log_callback callback){
    gfx_log = callback;
}

gfx_type gfx_get_type(GLenum type) {
    switch (type) {
        case GL_FLOAT: return GFX_TYPE_FLOAT_VEC_1;
        case GL_FLOAT_VEC2: return GFX_TYPE_FLOAT_VEC_2;
        case GL_FLOAT_VEC3: return GFX_TYPE_FLOAT_VEC_3;
        case GL_FLOAT_VEC4: return GFX_TYPE_FLOAT_VEC_4;
        case GL_INT: return GFX_TYPE_INTEGER_VEC_1;
        case GL_INT_VEC2: return GFX_TYPE_INTEGER_VEC_2;
        case GL_INT_VEC3: return GFX_TYPE_INTEGER_VEC_3;
        case GL_INT_VEC4: return GFX_TYPE_INTEGER_VEC_4;
        case GL_FLOAT_MAT3: return GFX_TYPE_FLOAT_MAT_3;
        case GL_FLOAT_MAT4: return GFX_TYPE_FLOAT_MAT_4;
        case GL_SAMPLER_CUBE: return GFX_TYPE_SAMPLER_CUBE;
        case GL_TEXTURE: return GFX_TYPE_TEXTURE;
    }

    CORE_ASSERT(0==1 && "Invalid gfx type");
    return 0;
}

char* gfx_get_type_char(GLenum type) {

    switch (type) {
        case GFX_TYPE_FLOAT_VEC_1: return "gfx_type_float_vec_1";
        case GFX_TYPE_FLOAT_VEC_2: return "gfx_type_float_vec_2";
        case GFX_TYPE_FLOAT_VEC_3: return "gfx_type_float_vec_3";
        case GFX_TYPE_FLOAT_VEC_4: return "gfx_type_float_vec_4";
        case GFX_TYPE_INTEGER_VEC_1: return "gfx_type_integer_vec_1";
        case GFX_TYPE_INTEGER_VEC_2: return "gfx_type_integer_vec_2";
        case GFX_TYPE_INTEGER_VEC_3: return "gfx_type_integer_vec_3";
        case GFX_TYPE_INTEGER_VEC_4: return "gfx_type_integer_vec_4";
        case GFX_TYPE_FLOAT_MAT_3: return "gfx_type_float_mat_3";
        case GFX_TYPE_FLOAT_MAT_4: return "gfx_type_float_mat_4";
        case GFX_TYPE_SAMPLER_CUBE: return "gfx_type_sampler_cube";
        case GFX_TYPE_TEXTURE: return "gfx_type_texture";
    }

    CORE_ASSERT(0==1 && "Invalid GLenum type");
    return "";
}

GLenum gl_get_type(gfx_type type){
    switch (type) {
        case GFX_TYPE_FLOAT_VEC_1: return GL_FLOAT;
        case GFX_TYPE_FLOAT_VEC_2: return GL_FLOAT_VEC2;
        case GFX_TYPE_FLOAT_VEC_3: return GL_FLOAT_VEC3;
        case GFX_TYPE_FLOAT_VEC_4: return GL_FLOAT_VEC4;
        case GFX_TYPE_INTEGER_VEC_1: return GL_INT;
        case GFX_TYPE_INTEGER_VEC_2: return GL_INT_VEC2;
        case GFX_TYPE_INTEGER_VEC_3: return GL_INT_VEC3;
        case GFX_TYPE_INTEGER_VEC_4: return GL_INT_VEC4;
        case GFX_TYPE_FLOAT_MAT_3: return GL_FLOAT_MAT3;
        case GFX_TYPE_FLOAT_MAT_4: return GL_FLOAT_MAT4;
        case GFX_TYPE_SAMPLER_CUBE: return GL_SAMPLER_CUBE;
        case GFX_TYPE_TEXTURE: return GL_TEXTURE;
        case GFX_TYPE_INVALID: return 0;
    }
    CORE_ASSERT(0==1 && "Invalid enum gfx type");
    return 0;
}

uint32_t gl_get_buffer_update_mode(gfx_buffer_update_mode mode)
{
    switch (mode) {
        case GFX_BUFFER_UPDATE_STATIC_DRAW: return GL_STATIC_DRAW;
        case GFX_BUFFER_UPDATE_STATIC_READ: return GL_STATIC_READ;
        case GFX_BUFFER_UPDATE_STATIC_COPY: return GL_STATIC_COPY;
        case GFX_BUFFER_UPDATE_DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
        case GFX_BUFFER_UPDATE_DYNAMIC_READ: return GL_DYNAMIC_READ;
        case GFX_BUFFER_UPDATE_DYNAMIC_COPY: return GL_DYNAMIC_COPY;
        case GFX_BUFFER_UPDATE_STREAM_DRAW: return GL_STREAM_DRAW;
        case GFX_BUFFER_UPDATE_STREAM_READ: return GL_STREAM_READ;
        case GFX_BUFFER_UPDATE_STREAM_COPY: return GL_STREAM_COPY;
        default:
            break;
    }

    CORE_ASSERT(0==1 && "Invalid buffer update mode enum type");
}

uint32_t gl_get_buffer_type(gfx_buffer_type type) {
    switch (type) {
        case GFX_BUFFER_VERTEX:
            return GL_ARRAY_BUFFER;
        case GFX_BUFFER_INDEX:
            return GL_ELEMENT_ARRAY_BUFFER;
        default:
            CORE_ASSERT(0 && "Invalid buffer type");
            return 0;
    }
}

void gl_print_shader_err(gfx_shader_handle handle, int32_t id)
{
    GLint maxLength = 0;
    char* buffer;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetShaderInfoLog(id, maxLength, &maxLength, buffer);
    LOG_ERROR("%s\n", buffer);
    OS_FREE(buffer);
}

void gl_print_program_err(gfx_shader_handle handle)
{
    GLint maxLength = 0;
    char* buffer;
    glGetProgramiv(handle->id, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetProgramInfoLog(handle->id, maxLength, &maxLength, buffer);
    LOG_ERROR("%s\n", buffer);
    OS_FREE(buffer);
}

void gfx_get_uniform_setter(void(**uniform_setter)(uint32_t index, void* data), gfx_type type){
    switch (type) {
        case GFX_TYPE_FLOAT_VEC_1: *uniform_setter = uniform_set_f1; break;
        case GFX_TYPE_FLOAT_VEC_2: *uniform_setter = uniform_set_f2; break;
        case GFX_TYPE_FLOAT_VEC_3: *uniform_setter = uniform_set_f3; break;
        case GFX_TYPE_FLOAT_VEC_4: *uniform_setter = uniform_set_f4; break;
        case GFX_TYPE_INTEGER_VEC_1: *uniform_setter = uniform_set_i1; break;
        case GFX_TYPE_INTEGER_VEC_2: *uniform_setter = uniform_set_i2; break;
        case GFX_TYPE_INTEGER_VEC_3: *uniform_setter = uniform_set_i3; break;
        case GFX_TYPE_INTEGER_VEC_4: *uniform_setter = uniform_set_i4; break;
        case GFX_TYPE_FLOAT_MAT_3: *uniform_setter = uniform_set_m3; break;
        case GFX_TYPE_FLOAT_MAT_4: *uniform_setter = uniform_set_m4; break;
        default: *uniform_setter = 0;
    }
}

/*
 * Shaders
 */

gfx_shader_handle gfx_shader_create(const char* name) {
    gfx_shader_handle handle = OS_MALLOC(sizeof(struct gfx_shader));
    LOG("Shader %s created\n", name);
    os_memset(handle, 0, sizeof(gfx_shader));
    handle->id = glCreateProgram();
    handle->status = GFX_RESOURCE_CREATED;
    os_memcpy(handle->name, name, strlen(name) + 1);
    SHADER_CHANGE_CALLBACK(handle);
    return handle;
}

void gfx_shader_add_vs(gfx_shader_handle handle, const char* vs){
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED && "Shader add vs failed");
    int32_t compiled = 0;
    handle->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(handle->vs, 1, &vs, 0);
    glCompileShader(handle->vs);
    glGetShaderiv(handle->vs, GL_COMPILE_STATUS, &compiled);
    handle->vs_data = OS_MALLOC(strlen(vs)+1);
    os_memcpy(handle->vs_data, vs, strlen(vs)+1);
    if (!compiled) {
        LOG_ERROR("Vertex stage error for shader: %s\n", handle->name);
        gl_print_shader_err(handle, handle->vs);
    }
    else {
        glAttachShader(handle->id, handle->vs);
        LOG("Vertex stage attached to %s\n", handle->name);
    }
}

void gfx_shader_add_fs(gfx_shader_handle handle, const char* fs){
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED && "Shader add fs failed");
    int32_t compiled = 0;
    handle->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(handle->fs, 1, &fs, 0);
    glCompileShader(handle->fs);
    glGetShaderiv(handle->fs, GL_COMPILE_STATUS, &compiled);
    handle->fs_data = OS_MALLOC(strlen(fs)+1);
    os_memcpy(handle->fs_data, fs, strlen(fs)+1);
    if (!compiled) {
        LOG_ERROR("Fragment stage error for shader: %s\n", handle->name);
        gl_print_shader_err(handle, handle->fs);
    }
    else {
        glAttachShader(handle->id, handle->fs);
        LOG("Fragment stage attached to %s\n", handle->name);
    }
}

void gfx_shader_submit(gfx_shader_handle handle) {
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED && "Shader submit failed");

    //compile program
    int32_t compiled = 0;
    glLinkProgram(handle->id);
    glGetProgramiv(handle->id, GL_LINK_STATUS, &compiled);

    //store active attributes
    GLenum attr_type;
    int32_t name_length;
    glGetProgramiv(handle->id, GL_ACTIVE_ATTRIBUTES, &handle->attr_count);
    for (int32_t i = 0; i < handle->attr_count; i++) {
        struct gfx_shader_attribute *attr = handle->attributes + i;
        glGetActiveAttrib(handle->id, (GLuint) i, MAXIMUM_ATTRIBUTE_NAME_LENGTH, &name_length, &attr->length,
                          &attr_type,
                          attr->name);

        attr->id = glGetAttribLocation(handle->id, attr->name);
        attr->enabled = true;
        attr->type = gfx_get_type(attr_type);

        LOG("Attribute uniform name: %s, type: %s, index: %i, length: %i\n", attr->name,
               gfx_get_type_char(attr->type), i, attr->length);

    }

    //store active uniforms
    glGetProgramiv(handle->id, GL_ACTIVE_UNIFORMS, &handle->uniform_count);
    for (int32_t i = 0; i < handle->uniform_count; i++) {
        struct gfx_shader_uniform *uniform = handle->uniforms + i;
        glGetActiveUniform(handle->id, (GLuint) i, MAXIMUM_UNIFORM_NAME_LENGTH, &name_length, &uniform->length,
                           &attr_type, uniform->name);
        uniform->id = glGetUniformLocation(handle->id, uniform->name);
        uniform->enabled = true;
        uniform->type = gfx_get_type(attr_type);
        LOG("Shader uniform name: %s, type: %s, index: %i, length: %i\n", uniform->name,
               gfx_get_type_char(uniform->type), i, uniform->length);

        //cache uniform setter
        gfx_get_uniform_setter(&uniform->uniform_setter, uniform->type);
    }


    if (compiled) {
        LOG("Shader program compiled: %s\n", handle->name);
        handle->status = GFX_RESOURCE_ACTIVE;
        SHADER_CHANGE_CALLBACK(handle);
    } else {
        handle->status = GFX_RESOURCE_INVALID;
        SHADER_CHANGE_CALLBACK(handle);
        LOG_ERROR("Shader program error: %s\n", handle->name);
        gl_print_program_err(handle);
    }

    if (compiled) {
        int32_t count = 0;
        handle->uniform_commands_count = 0;
        for (int32_t i = 0; i < count; ++i) {
            struct gfx_shader_uniform_command cmd = handle->uniform_commands[i];
            gfx_shader_uniform_enable(handle, cmd.name, cmd.type, cmd.reference_index);
        }
    }

    for (int32_t i = 0; i < handle->pipeline_count; ++i) {
        struct gfx_pipeline *pip = handle->pipelines[i];
        int32_t commands_count = pip->attrs_commands_count;
        pip->attrs_commands_count = 0;

        gfx_pipeline_reload(pip);

        if (pip->ebo_command.enabled)
            gfx_pipeline_index_enable(pip, pip->ebo_command.buffer);

        for (int32_t j = 0; j < commands_count; ++j) {
            struct gfx_pipeline_attr_command command = pip->attr_commands[j];
            gfx_pipeline_attr_enable(pip, command.name, command.buffer, command.count, command.offset,
                                     command.stride);
        }

        gfx_pipeline_submit(pip);
    }


    //free shader objects
    glDeleteShader(handle->fs);
    glDeleteShader(handle->vs);

    handle->fs = 0;
    handle->vs = 0;
}

void gfx_shader_destroy(gfx_shader_handle handle) {
    CORE_ASSERT(handle->status != GFX_RESOURCE_DESTROYED && "Destroy failed");
    handle->status = GFX_RESOURCE_DESTROYED;
    SHADER_CHANGE_CALLBACK(handle);

    glDeleteProgram(handle->id);
    handle->id = GL_INVALID_INDEX;
    OS_FREE(handle->vs_data);
    OS_FREE(handle->fs_data);
    LOG("Shader destroyed: %s\n", handle->name);
    OS_FREE(handle);

}

void gfx_shader_reload(gfx_shader_handle handle){
    CORE_ASSERT(handle->status != GFX_RESOURCE_DESTROYED && "Reload failed");
    handle->status = GFX_RESOURCE_CREATED;
    handle->uniform_count = 0;
    handle->attr_count = 0;
    glDeleteProgram(handle->id);
    handle->id = glCreateProgram();
    OS_FREE(handle->vs_data);
    OS_FREE(handle->fs_data);
    SHADER_CHANGE_CALLBACK(handle);
    LOG("Shader reloaded: %s\n", handle->name);
}

void gfx_shader_status_change_callback_set(shader_change_callback callback) {
    shader_change = callback;
}

void gfx_shader_fs_get(gfx_shader_handle handle, char ** ptr) {
    *ptr = handle->fs_data;
}

void gfx_shader_vs_get(gfx_shader_handle handle, char ** ptr){
    *ptr = handle->vs_data;
}

void gfx_shader_name_get(gfx_shader_handle handle, char ** ptr){
    *ptr = handle->name;
}

gfx_resource_status gfx_shader_status(gfx_shader_handle handle) {
    return handle->status;
}

void gfx_shader_bind(gfx_shader_handle shader) {
    if(shader->status == GFX_RESOURCE_ACTIVE) {
        glUseProgram(shader->id);
    }
    else{
        glUseProgram(0);
    }
}

/*
 * Buffers
 */

gfx_buffer_handle gfx_buffer_create(enum gfx_buffer_type type, enum gfx_buffer_update_mode mode, void* data, int32_t size) {
    gfx_buffer_handle buffer = OS_MALLOC(sizeof(struct gfx_buffer));
    buffer->status = GFX_RESOURCE_ACTIVE;
    buffer->type = type;
    buffer->mode = mode;

    glGenBuffers(1, &(buffer->id));

    uint32_t update_mode = gl_get_buffer_update_mode(mode);
    uint32_t target = gl_get_buffer_type(type);

    glBindBuffer(target, buffer->id);
    glBufferData(target, size, data, update_mode);
    return buffer;
}

void gfx_buffer_update(gfx_buffer_handle handle, void* data, int32_t offset, int32_t size) {
    CORE_ASSERT(handle->status == GFX_RESOURCE_ACTIVE);
    glBindBuffer(gl_get_buffer_type(handle->type), handle->id);
    glBufferSubData(gl_get_buffer_type(handle->type), offset, size, data);
}

void gfx_buffer_destroy(gfx_buffer_handle buffer) {
    buffer->status = GFX_RESOURCE_DESTROYED;
    glDeleteBuffers(1, &(buffer->id));
    OS_FREE(buffer);
}

gfx_resource_status gfx_buffer_status(gfx_buffer_handle handle) {
    return handle->status;
}

/*
 * Draw
 */

void gfx_enable_pass(gfx_draw_pass const* pass)
{
    if(pass->fbo != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pass->fbo->id);
    }
    else{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    if((pass->option & GFX_PASS_OPTION_DEPTH_TEST) != 0)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if((pass->option & GFX_PASS_OPTION_CULL_FRONT) != 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
    else if((pass->option & GFX_PASS_OPTION_CULL_BACK) != 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else{
        glDisable(GL_CULL_FACE);
    }

    if((pass->action & GFX_PASS_ACTION_CLEAR_COLOR) != 0) {
        glClearColor(pass->color[0], pass->color[1], pass->color[2], pass->color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    if((pass->action & GFX_PASS_ACTION_CLEAR_DEPTH) != 0) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    if((pass->action & GFX_PASS_ACTION_CLEAR_STENCIL) != 0) {
        glClear(GL_STENCIL_BUFFER_BIT);
    }

}

void gfx_begin_pass(gfx_framebuffer_handle handle, enum gfx_pass_option option, enum gfx_pass_action action, float color[4]) {

    gfx_draw_pass pass = {.option = option, .action = action, .fbo = handle, .draw_calls = 0};
    os_memcpy(pass.color, color, sizeof(float) * 4);
    draw_pass_array[draw_pass_count] = pass;
    gfx_enable_pass(&pass);
    draw_pass_count++;
}

int32_t gfx_end_pass() {

    CORE_ASSERT(draw_pass_count != 0 && "Trying to end non existing draw pass");

    gfx_draw_pass pass = draw_pass_array[draw_pass_count - 1];
    draw_pass_count--;

    if (draw_pass_count != 0) {
        gfx_draw_pass new_pass = draw_pass_array[draw_pass_count - 1];
        gfx_enable_pass(&new_pass);
    }

    return pass.draw_calls;
}

void gfx_draw(gfx_draw_type type, int32_t start, int32_t length) {
    CORE_ASSERT(draw_pass_count != 0 && "Drawing while no draw pass is active");

    gfx_draw_pass pass = draw_pass_array[draw_pass_count-1];
    glDrawArrays(type, start, length);
    pass.draw_calls++;
}

void gfx_draw_id(gfx_draw_type type, int32_t length)
{
    CORE_ASSERT(draw_pass_count != 0 && "Drawing while no draw pass is active");

    gfx_draw_pass pass = draw_pass_array[draw_pass_count-1];
    glDrawElements(type, length, GL_UNSIGNED_INT, 0);
    pass.draw_calls++;
}

void gfx_viewport_set(int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
}

/*
 * Pipeline
 */

gfx_pipeline_handle gfx_pipeline_create(gfx_shader_handle handle) {

    LOG("Pipeline created, shader: %s\n", handle->name);

    gfx_pipeline_handle pipeline = OS_MALLOC(sizeof(struct gfx_pipeline));
    os_memset(pipeline, 0, sizeof(struct gfx_pipeline));

    pipeline->shader_handle = handle;
    pipeline->ebo_id = GL_INVALID_INDEX;
    pipeline->status = GFX_RESOURCE_CREATED;

    CORE_ASSERT(handle != 0 && "Null shader handle provided");
    CORE_ASSERT(handle->pipeline_count != MAXIMUM_PIPELINES_PER_SHADER &&
                "Reached pipeline per shader capacity");

    pipeline->shader_pipeline_index = handle->pipeline_count;
    handle->pipelines[handle->pipeline_count] = pipeline;
    handle->pipeline_count++;

    glGenVertexArrays(1, &(pipeline->vao_id));
    return pipeline;
}

void gfx_pipeline_index_enable(gfx_pipeline_handle handle, gfx_buffer_handle buffer){
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED);
    glBindVertexArray(handle->vao_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->id);
    handle->ebo_id = buffer->id;
    handle->ebo_command.buffer = buffer;
    handle->ebo_command.enabled = true;
    glBindVertexArray(0);
    LOG("Pipeline EBO set %s\n", handle->shader_handle->name);
}

void gfx_pipeline_attr_enable(gfx_pipeline_handle handle, const char* name, gfx_buffer_handle buffer,
                              int32_t count, int32_t offset, int32_t stride) {

    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED);
    CORE_ASSERT(handle != 0 && "Attribute enable failed, null handle");
    CORE_ASSERT(name != 0 && "Attribute enable failed, invalid attribute name");
    CORE_ASSERT(buffer != 0 && "Attribute enable failed, invalid buffer");
    CORE_ASSERT(count != 0 && "Attribute enable failed, element count must be greater than zero");
    CORE_ASSERT(stride != 0 && "Attribute enable failed, stride must be greater than zero");

    os_memcpy(handle->attr_commands[handle->attrs_commands_count].name, name, strlen(name) + 1);
    handle->attr_commands[handle->attrs_commands_count].buffer = buffer;
    handle->attr_commands[handle->attrs_commands_count].count = count;
    handle->attr_commands[handle->attrs_commands_count].offset = offset;
    handle->attr_commands[handle->attrs_commands_count].stride = stride;
    handle->attrs_commands_count++;

    for (int32_t i = 0; i < handle->shader_handle->attr_count; ++i) {
        struct gfx_shader_attribute attr = handle->shader_handle->attributes[i];
        CORE_ASSERT(attr.enabled && "Attribute enable failed, this should not happen");

        if (strcmp(attr.name, name) == 0) {

            glBindVertexArray(handle->vao_id);
            glUseProgram(handle->shader_handle->id);
            glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
            glEnableVertexAttribArray(attr.id);
            glVertexAttribPointer(attr.id, count, GL_FLOAT, GL_FALSE, stride,
                                  (const void *) offset);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            LOG("Pipeline attribute enabled, shader %s, name: %s, type:%s, stride: %i, offset: %i, count: %i\n",
                handle->shader_handle->name, attr.name, gfx_get_type_char(attr.type), stride, offset, count);
            break;
        }
    }
}


void gfx_shader_uniform_enable(gfx_shader_handle handle, const char* name, gfx_type type, int32_t* uniform_index){

    CORE_ASSERT(handle->status == GFX_RESOURCE_ACTIVE);
    CORE_ASSERT(handle->uniform_commands_count != MAXIMUM_UNIFORM_COMMANDS_PER_SHADER);

    os_memcpy(handle->uniform_commands[handle->uniform_commands_count].name, name, strlen(name) + 1);
    handle->uniform_commands[handle->uniform_commands_count].type = type;
    handle->uniform_commands[handle->uniform_commands_count].reference_index = uniform_index;
    handle->uniform_commands_count++;

    for (int32_t i = 0; i < handle->uniform_count; ++i) {
        gfx_shader_uniform *uni = handle->uniforms + i;
        if (strcmp(uni->name, name) == 0) {
            CORE_ASSERT(uni->type == type);
            *uniform_index = i;
            LOG("Uniform enabled, shader: %s, name: %s, type:%s\n", handle->name,
                   name, gfx_get_type_char(uni->type));
            return;
        }
    }
    *uniform_index = -1;
}


void gfx_pipeline_use_depth_buffer(gfx_pipeline_handle handle, bool value) {
    handle->disable_depth_buffer = !value;
}

void gfx_pipeline_submit(gfx_pipeline_handle handle) {
    LOG("Pipeline submit, shader %s\n", handle->shader_handle->name);
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED);
    handle->status = GFX_RESOURCE_ACTIVE;
}

void gfx_pipeline_bind(gfx_pipeline_handle handle) {
    if(handle->status == GFX_RESOURCE_ACTIVE) {
        glUseProgram(handle->shader_handle->id);
        glBindVertexArray(handle->vao_id);
        if (handle->ebo_id != GL_INVALID_INDEX)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle->ebo_id);
        glDepthMask(!handle->disable_depth_buffer);
    }
    else {
        LOG_ERROR("Pipeline invalid, could not bind: %i\n", handle->status);
    }
}

void gfx_pipeline_reload(gfx_pipeline_handle handle) {
    LOG("Pipeline reloaded, shader: %s", handle->shader_handle->name);
    CORE_ASSERT(handle->status != GFX_RESOURCE_DESTROYED);
    glDeleteVertexArrays(1, &handle->vao_id);
    glCreateVertexArrays(1, &handle->vao_id);
    handle->ebo_id = GL_INVALID_INDEX;
    handle->status = GFX_RESOURCE_CREATED;
}

void gfx_pipeline_destroy(gfx_pipeline_handle handle) {
    CORE_ASSERT(handle->status != GFX_RESOURCE_DESTROYED);
    glDeleteVertexArrays(1, &(handle->vao_id));

    //swap remove pipeline from the shader
    if(handle->shader_handle != 0) {
        handle->shader_handle->pipelines[handle->shader_pipeline_index] = handle->shader_handle->pipelines[
                handle->shader_handle->pipeline_count - 1];
        handle->shader_handle->pipelines[handle->shader_pipeline_index]->shader_pipeline_index = handle->shader_pipeline_index;
        handle->shader_handle->pipeline_count--;

        LOG("Pipeline destroyed, shader: %s", handle->shader_handle->name);
    }
    else{
        LOGG_ERROR("Pipeline destroyed, but shader handle was destroyed before\n");
    }

    handle->status = GFX_RESOURCE_DESTROYED;
    OS_FREE(handle);
}

void gfx_shader_uniform_set(gfx_shader_handle handle, uint32_t uniform_index, void* data) {
    if(handle->status == GFX_RESOURCE_ACTIVE) {
        if (uniform_index != -1) {
            struct gfx_shader_uniform uni = handle->uniforms[uniform_index];
            uni.uniform_setter(uni.id, data);
        }
    }
}

/*
 * Blend
 */

uint32_t gfx_blend_type_to_gl(gfx_blend_type type) {
    switch (type) {

        case GFX_BLEND_SRC_ALPHA: return GL_BLEND_SRC_ALPHA;
        case GFX_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case GFX_BLEND_SRC_COLOR: return GL_SRC_COLOR;
        case GFX_BLEND_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case GFX_BLEND_ONE: return GL_ONE;
        case GFX_BLEND_ZERO: return GL_ZERO;
        case GFX_BLEND_DEST_ALPHA: return GL_BLEND_DST_ALPHA;
        case GFX_BLEND_ONE_MINUS_DEST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case GFX_BLEND_DEST_COLOR: return GL_DST_COLOR;
        case GFX_BLEND_ONE_MINUS_DEST_COLOR: return GL_ONE_MINUS_DST_COLOR;
    }

    CORE_ASSERT(0 == 1 && "Invalid blend type");
    return GL_ZERO;
}

void gfx_blend( gfx_blend_type src, gfx_blend_type dest) {
    glBlendFunc(gfx_blend_type_to_gl(src), gfx_blend_type_to_gl(dest));
}

void gfx_blend_enable(bool state) {
    if(state) glEnable(GL_BLEND);
    else glDisable(GL_BLEND);
}

/*
 * Framebuffer
 */

gfx_framebuffer_handle gfx_framebuffer_create(gfx_texture_handle color_texture, gfx_texture_handle depth_texture) {

    gfx_framebuffer_handle handle = OS_MALLOC(sizeof(struct gfx_framebuffer));
    handle->status = GFX_RESOURCE_CREATED;

    glGenFramebuffers(1, &(handle->id));
    glBindFramebuffer(GL_FRAMEBUFFER, handle->id);

    if (color_texture) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->id, 0);
    }

    if (depth_texture) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture->id, 0);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        handle->status = GFX_RESOURCE_ACTIVE;
        LOGG("Fbo created\n");
    }
    else {
        handle->status = GFX_RESOURCE_INVALID;
        LOGG_ERROR("Fbo invalid\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return handle;
}

void gfx_framebuffer_destroy(gfx_framebuffer_handle handle) {
    LOGG("Fbo destroyed\n");
    CORE_ASSERT(handle->status == GFX_RESOURCE_CREATED || handle->status == GFX_RESOURCE_ACTIVE);
    glDeleteFramebuffers(1, &handle->id);
    OS_FREE(handle);
}


/*
 * Texture related.
 */


void gfx_texture_gl_apply_filter(enum gfx_texture_filter_mode mode, bool mipmaps) {
    switch (mode) {
        case GFX_TEXTURE_FILTER_LINEAR:
            if (mipmaps) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            break;
        case GFX_TEXTURE_FILTER_NEAREST:
            if (mipmaps) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            break;
    }
}

void gfx_texture_gl_apply_wrap(enum gfx_texture_wrap_mode mode){

    switch (mode) {

        case GFX_TEXTURE_WRAP_CLAMP:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        case GFX_TEXTURE_WRAP_REPEAT:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        case GFX_TEXTURE_WRAP_MIRROR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            break;
    }
}

void gfx_texture_gl_get_type(enum gfx_texture_type type, int32_t* tex_type_src, int32_t* tex_type_dest, int32_t* format){
    switch (type) {
        case GFX_TEXTURE_TYPE_SRGBA:
            *tex_type_src = GL_SRGB_ALPHA;
            *tex_type_dest = GL_RGBA;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_SRGB:
            *tex_type_src = GL_SRGB;
            *tex_type_dest = GL_RGB;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_RGB:
            *tex_type_src = GL_RGB;
            *tex_type_dest = GL_RGB;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_RGB16:
            *tex_type_src = GL_RGB16F;
            *tex_type_dest = GL_RGB;
            *format = GL_FLOAT;
            break;
        case GFX_TEXTURE_TYPE_RGBA:
            *tex_type_src = GL_RGBA;
            *tex_type_dest = GL_RGBA;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_DEPTH:
            *tex_type_src = GL_DEPTH_COMPONENT;
            *tex_type_dest = GL_DEPTH_COMPONENT;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_STENCIL:
            *tex_type_src = GL_STENCIL_INDEX8;
            *tex_type_dest = GL_STENCIL_INDEX;
            *format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_DEPTH_STENCIL:
            *tex_type_src = GL_DEPTH24_STENCIL8;
            *tex_type_dest = GL_DEPTH_STENCIL;
            *format = GL_UNSIGNED_INT_24_8;
            break;
    }
}

gfx_texture_handle gfx_texture_create(int32_t width, int32_t height, void* data, enum gfx_texture_type type, enum gfx_texture_filter_mode filter, enum gfx_texture_wrap_mode wrap){

    LOG("Texture created, width:%i, height:%i \n",width, height);

    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int32_t tex_type_src, tex_type_dest, format;
    gfx_texture_gl_get_type(type, &tex_type_src, &tex_type_dest, &format);
    glTexImage2D(GL_TEXTURE_2D, 0, tex_type_src, width, height, 0, tex_type_dest, format, data);

    gfx_texture_gl_apply_filter(filter, false);
    gfx_texture_gl_apply_wrap(wrap);

    glBindTexture(GL_TEXTURE_2D, 0);

    gfx_texture_handle hndl = OS_MALLOC(sizeof(gfx_texture));
    hndl->width = width;
    hndl->height = height;
    hndl->id = texture;
    hndl->filter = filter;
    hndl->wrap = wrap;
    hndl->type = type;
    return hndl;
}

gfx_texture_handle gfx_texture_load(const char* path, enum gfx_texture_type type, enum gfx_texture_filter_mode filter, enum gfx_texture_wrap_mode wrap) {

    int32_t width, height, channels;
    void* src = stbi_load(path, &width, &height, &channels, 0);
    if (src == 0) {
        LOG_ERROR("Texture could not be loaded: %s", path);
        return 0;
    }
    LOG("Image loaded, path: %s, width:%i, height:%i \n", path, width, height);
    gfx_texture_handle handle = gfx_texture_create(width, height, src, type, filter, wrap);
    stbi_image_free(src);
    return handle;
}

 int32_t gfx_texture_get_id(gfx_texture_handle handle){
    return handle->id;
}

gfx_texture_cubemap_handle gfx_texture_cubemap_create(const char* path, const char* names[6], enum gfx_texture_type type, enum gfx_texture_filter_mode filter){
    uint32_t id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    int32_t tex_type_src, tex_type_dest, format;
    gfx_texture_gl_get_type(type, &tex_type_src, &tex_type_dest, &format);

    for(int32_t i = 0; i < 6; i++) {
        int32_t width, height, ch;
        void *src;

        char buffer[MAXIMUM_PATH_LENGTH];
        sprintf(buffer, "%s%s", path, names[i]);
        src = stbi_load(buffer, &width, &height, &ch, 0);

        if(src == 0) {
            LOG_ERROR("Cubemap load fail, side %i, path: %s\n", i, buffer);
        }

        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, tex_type_src, width, height, 0, tex_type_dest, format,
                src
        );

        stbi_image_free(src);
    }

    LOG("Cubemap loaded, path: %s\n", path);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    gfx_texture_cubemap_handle hndl = OS_MALLOC(sizeof(gfx_texture_cubemap));
    hndl->texture.id = id;
    return hndl;
}

void gfx_texture_bind(gfx_texture_handle hndl, int32_t index) {

    if (hndl != 0) {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, hndl->id);
    }
    else{
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void gfx_texture_destroy(gfx_texture_handle hndl) {
    if(hndl == 0) return;
    glDeleteTextures(1, &(hndl->id));
    OS_FREE(hndl);
    LOGG("Texture destroyed\n");
}

void gfx_texture_cubemap_bind(gfx_texture_cubemap_handle hndl) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, hndl->texture.id);
}

void gfx_texture_cubemap_destroy(gfx_texture_cubemap_handle hndl) {
    glDeleteTextures(1, &hndl->texture.id);
    OS_FREE(hndl);
    LOGG("Cubemap destroyed\n");
}
