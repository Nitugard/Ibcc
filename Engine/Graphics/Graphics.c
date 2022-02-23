/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <GL/gl3w.h>

#include "Graphics.h"

#include <Os/Log.h>
#include <Os/Allocator.h>
#include <Os/Platform.h>
#include <Containers/Array.h>

typedef struct gfx_shader{
    int32_t id;
    gfx_shader_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
} gfx_shader;

typedef struct gfx_buffer{
    uint32_t id;
} gfx_buffer;

typedef struct gfx_pipeline{
    bool smooth_model;
    gfx_shader_handle shader;
    uint32_t vao_id;
    uint32_t ebo_id;
}gfx_pipeline;

typedef struct gfx_texture{
    uint32_t id;
    int32_t width;
    int32_t height;
    gfx_texture_filter_mode filter;
    gfx_texture_wrap_mode wrap;
    gfx_texture_type type;
    bool mipmaps;
} gfx_texture;

typedef struct gfx_framebuffer{
    uint32_t id;
} gfx_framebuffer;


typedef struct gfx_draw_pass{
    gfx_pass_desc desc;
    int32_t draw_calls;
} gfx_draw_pass;

arr_handle draw_passes_arr;
int32_t total_draw_calls;

void
#ifdef __MINGW32__
__attribute__((stdcall))
#endif
opengl_msg_callback( GLenum source,GLenum type, GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam )
{

    LOG_ERROR("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

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


bool gfx_init() {
    if (gl3wInit()) {
        LOG_ERROR("Failed to initialize OpenGL\n");
        return false;
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEBUG_OUTPUT);
    glFrontFace(GL_CCW);
    glDebugMessageCallback((GLDEBUGPROC) opengl_msg_callback, 0);

    LOG_INFO("Graphics initialized\n");
    LOG_INFO("%s %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    draw_passes_arr = arr_new(sizeof(struct gfx_draw_pass), 0);
    total_draw_calls = 0;

    return true;
}

void gfx_terminate(){
    arr_delete(draw_passes_arr);
}


uint32_t gl_get_buffer_update_mode(gfx_buffer_update_mode mode)
{
    switch (mode) {
        case GFX_BUFFER_UPDATE_STATIC_DRAW:
            return GL_STATIC_DRAW;
        case GFX_BUFFER_UPDATE_STATIC_READ:
            return GL_STATIC_READ;
        case GFX_BUFFER_UPDATE_STATIC_COPY:
            return GL_STATIC_COPY;
        case GFX_BUFFER_UPDATE_DYNAMIC_DRAW:
            return GL_DYNAMIC_DRAW;
        case GFX_BUFFER_UPDATE_DYNAMIC_READ:
            return GL_DYNAMIC_READ;
        case GFX_BUFFER_UPDATE_DYNAMIC_COPY:
            return GL_DYNAMIC_COPY;
        case GFX_BUFFER_UPDATE_STREAM_DRAW:
            return GL_STREAM_DRAW;
        case GFX_BUFFER_UPDATE_STREAM_READ:
            return GL_STREAM_READ;
        case GFX_BUFFER_UPDATE_STREAM_COPY:
            return GL_STREAM_COPY;
        default:
            break;
    }
    ASSERT(0==1);
}

uint32_t gl_get_buffer_type(gfx_buffer_type type) {
    switch (type) {
        case GFX_BUFFER_VERTEX:
            return GL_ARRAY_BUFFER;
        case GFX_BUFFER_INDEX:
            return GL_ELEMENT_ARRAY_BUFFER;
        case GFX_BUFFER_UNIFORM:
            return GL_UNIFORM_BUFFER;
        default:
            ASSERT(0 && "Invalid buffer type");
            return 0;
    }
}

void gl_print_shader_err(int32_t shader, char const* name)
{
    GLint maxLength = 0;
    char* buffer;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buffer);
    LOG_ERROR("Shader %s compilation error: \n%s", name, buffer);
    OS_FREE(buffer);
}

void gl_print_program_err(int32_t shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetProgramInfoLog(shader, maxLength, &maxLength, buffer);
    LOG_ERROR("Program linkage error: \n%s", buffer);
    OS_FREE(buffer);
}



gfx_shader_handle gfx_shader_create(const gfx_shader_desc *desc) {

    gfx_shader_handle shader = OS_MALLOC(sizeof(gfx_shader));

    shader->id = glCreateProgram();
    memcpy(shader->attrs, desc->attrs, sizeof(struct gfx_shader_attr) * MAXIMUM_PIPELINE_ATTRIBUTES);

    int32_t compiled = 0;
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &desc->vs.src, 0);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (compiled) {
        glAttachShader(shader->id, vs);
    }
    else {
        gl_print_shader_err(vs, desc->name);
    }
    compiled = 0;
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &desc->fs.src, 0);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, fs);
    else {
        gl_print_shader_err(fs, desc->name);
    }

    compiled = 0;
    glLinkProgram(shader->id);
    glGetProgramiv(shader->id, GL_LINK_STATUS, &compiled);

    if (compiled) {
        LOG_INFO("Shader created: %s, id: %i\n", desc->name, shader->id);
    }else gl_print_program_err(shader->id);
    glDeleteShader(fs);
    glDeleteShader(vs);

    return shader;
}

void gfx_shader_destroy(gfx_shader_handle shader) {
    LOG_INFO("Shader destroyed, id: %i\n", shader->id);
    glDeleteProgram(shader->id);
    OS_FREE(shader);
}

gfx_buffer_handle gfx_buffer_create(const gfx_buffer_desc * desc) {

    if(desc->type == GFX_BUFFER_UNIFORM && desc->size >= GL_MAX_UNIFORM_BLOCK_SIZE) {
        LOG_ERROR("Failed to create shader. Uniform block size reached its limit!");
        return 0;
    }

    ASSERT(desc->size != 0);
    gfx_buffer_handle buffer = OS_MALLOC(sizeof(struct gfx_buffer));

    glGenBuffers(1, &(buffer->id));
    uint32_t target, update_mode;
    update_mode = gl_get_buffer_update_mode(desc->update_mode);
    target = gl_get_buffer_type(desc->type);

    glBindBuffer(target, buffer->id);
    glBufferData(target, desc->size, desc->data, update_mode);

    return buffer;
}

void gfx_buffer_destroy(gfx_buffer_handle buffer) {
    glDeleteBuffers(1, &(buffer->id));
    OS_FREE(buffer);
}

void gfx_buffer_update(gfx_buffer_handle buffer, const gfx_buffer_desc * desc) {
    glBindBuffer(gl_get_buffer_type(desc->type), buffer->id);
    glBufferSubData(gl_get_buffer_type(desc->type), 0, desc->size, desc->data);
}

void gfx_draw(gfx_draw_type type, int32_t start, int32_t length) {
    int32_t size = arr_size(draw_passes_arr);
    if (size == 0) {
        LOG_ERROR("Draw pass is not active\n");
        return;
    }

    gfx_draw_pass *pass = arr_get(draw_passes_arr, size - 1);
    glDrawArrays(type, start, length);
    pass->draw_calls++;
}

void gfx_draw_id(gfx_draw_type type, int32_t length)
{
    int32_t size = arr_size(draw_passes_arr);
    if (size == 0) {
        LOG_ERROR("Draw pass is not active\n");
        return;
    }

    gfx_draw_pass *pass = arr_get(draw_passes_arr, size - 1);
    glDrawElements(type, length, GL_UNSIGNED_INT, 0);
    pass->draw_calls++;
}

gfx_pipeline_handle gfx_pipeline_create(const gfx_pipeline_desc *desc) {

    gfx_pipeline_handle pipeline = OS_MALLOC(sizeof(struct gfx_pipeline));
    pipeline->shader = desc->shader;
    glCreateVertexArrays(1, &(pipeline->vao_id));
    glBindVertexArray(pipeline->vao_id);

    if(desc->index_buffer != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, desc->index_buffer->id);
        pipeline->ebo_id = desc->index_buffer->id;
    }
    else {pipeline->ebo_id = GL_INVALID_INDEX;}

    for(int i=0; i < MAXIMUM_PIPELINE_ATTRIBUTES; ++i) {
        gfx_pipeline_attr p_attr = desc->attrs[i];
        if (p_attr.enabled) {
            //assert whether specification matches shader specs
            ASSERT(p_attr.element_size == desc->shader->attrs[i].size);
            ASSERT(p_attr.elements_count == desc->shader->attrs[i].num_elements);
            ASSERT(p_attr.buffer != 0);
            ASSERT(p_attr.stride != 0);
            ASSERT(p_attr.offset < p_attr.stride);

            gfx_shader_attr s_attr = desc->shader->attrs[i];

            glBindBuffer(GL_ARRAY_BUFFER, p_attr.buffer->id);
            glEnableVertexAttribArray(i);

            glVertexAttribPointer(i, s_attr.num_elements, GL_FLOAT, GL_FALSE, p_attr.stride,
                                  (const void *) p_attr.offset);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    glBindVertexArray(0);
    return pipeline;
}

void gfx_pipeline_bind(gfx_pipeline_handle pip) {
    glUseProgram(pip->shader->id);
    glBindVertexArray(pip->vao_id);

    if (pip->ebo_id != GL_INVALID_INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pip->ebo_id);
}


void gfx_pipeline_destroy(gfx_pipeline_handle hndl) {
    glDeleteVertexArrays(1, &(hndl->vao_id));
    OS_FREE(hndl);
}

void gfx_enable_pass(const gfx_pass_desc * desc)
{

    if(desc->fbo_handle != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, desc->fbo_handle->id);

    }
    else{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    uint32_t clear_flags = 0;
    if((desc->actions & GFX_PASS_ACTION_CLEAR_COLOR) != 0)
        clear_flags |= GL_COLOR_BUFFER_BIT;
    if((desc->actions & GFX_PASS_ACTION_CLEAR_DEPTH) != 0)
        clear_flags |= GL_DEPTH_BUFFER_BIT;
    if((desc->actions & GFX_PASS_ACTION_CLEAR_STENCIL) != 0)
        clear_flags |= GL_STENCIL_BUFFER_BIT;

    if (clear_flags != 0) {
        glClearColor(desc->clear_color.x, desc->clear_color.y, desc->clear_color.z, desc->clear_color.w);
        glClear(clear_flags);
    }

    if((desc->pass_options & GFX_PASS_OPTION_DEPTH_TEST) != 0)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if((desc->pass_options & GFX_PASS_OPTION_CULL_FRONT) != 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
    else if((desc->pass_options & GFX_PASS_OPTION_CULL_BACK) != 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else{
        glDisable(GL_CULL_FACE);
    }

    if((desc->pass_options & GFX_PASS_OPTION_FRAMEBUFFER_SRGB) != 0) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else
        glDisable(GL_FRAMEBUFFER_SRGB);
}

void gfx_begin_pass(const gfx_pass_desc * desc) {

    gfx_draw_pass pass = {.desc = *desc, .draw_calls = 0};
    arr_add(draw_passes_arr, &pass);
    gfx_enable_pass(desc);
}

int32_t gfx_end_pass() {

    if (arr_size(draw_passes_arr) == 0) {
        LOG_ERROR("Tried to end non existing pass");
        return 0;
    }
    struct gfx_draw_pass* dp = (gfx_draw_pass*)arr_get_last(draw_passes_arr );
    arr_remove_last(draw_passes_arr);
    if (arr_size(draw_passes_arr) != 0) {
        struct gfx_draw_pass *np = (gfx_draw_pass *) arr_get_last(draw_passes_arr);
        gfx_enable_pass(&(np->desc));
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return dp->draw_calls;
}

gfx_texture_handle gfx_texture_create(const gfx_texture_desc * desc) {

    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);


    uint32_t tex_type_src, tex_type_dest, format;
    switch (desc->type) {
        case GFX_TEXTURE_TYPE_SRGBA:
            tex_type_src = GL_SRGB_ALPHA;
            tex_type_dest = GL_RGBA;
            format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_SRGB:
            tex_type_src = GL_SRGB;
            tex_type_dest = GL_RGB;
            format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_RGB:
            tex_type_src = GL_RGB;
            tex_type_dest = GL_RGB;
            format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_RGBA:
            tex_type_src = GL_RGBA;
            tex_type_dest = GL_RGBA;
            format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_DEPTH:
            tex_type_src = GL_DEPTH_COMPONENT;
            tex_type_dest = GL_DEPTH_COMPONENT;
            format = GL_FLOAT;
            break;
        case GFX_TEXTURE_TYPE_STENCIL:
            tex_type_src = GL_STENCIL_INDEX8;
            tex_type_dest = GL_STENCIL_INDEX;
            format = GL_UNSIGNED_BYTE;
            break;
        case GFX_TEXTURE_TYPE_DEPTH_STENCIL:
            tex_type_src = GL_DEPTH24_STENCIL8;
            tex_type_dest = GL_DEPTH_STENCIL;
            format = GL_UNSIGNED_INT_24_8;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, tex_type_src, desc->width, desc->height, 0, tex_type_dest, format,
                 (unsigned char *) desc->data);


    switch (desc->wrap) {

        case GFX_TEXTURE_WRAP_CLAMP:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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


    if (desc->mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    switch (desc->filter) {
        case GFX_TEXTURE_FILTER_LINEAR:
            if(desc->mipmaps) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            break;
        case GFX_TEXTURE_FILTER_NEAREST:
            if(desc->mipmaps) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            else{
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            break;
    }


    glBindTexture(GL_TEXTURE_2D, 0);

    gfx_texture_handle hndl = OS_MALLOC(sizeof(gfx_texture));
    hndl->width = desc->width;
    hndl->height = desc->height;
    hndl->id = texture;
    hndl->filter = desc->filter;
    hndl->wrap = desc->wrap;
    hndl->type = desc->type;
    hndl->mipmaps = desc->mipmaps;
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
    glDeleteTextures(1, &(hndl->id));
    OS_FREE(hndl);
}

uint32_t gfx_draw_call_count_get() {
    return total_draw_calls;
}

gfx_uniform gfx_uniform_register(gfx_shader_handle shader, const char *name, gfx_type type) {
    gfx_uniform result = {0};
    glUseProgram(shader->id);
    result.id = glGetUniformLocation(shader->id, name);
    result.enabled = true;
    if(result.id == -1) {
        result.enabled = false; LOG_ERROR("Uniform not found %s for shader %i\n", name, shader->id);
        return result;
    }
    switch (type) {
        case GFX_FLOAT1: result.setter = uniform_set_f1; break;
        case GFX_FLOAT2: result.setter = uniform_set_f2; break;
        case GFX_FLOAT3: result.setter = uniform_set_f3; break;
        case GFX_FLOAT4: result.setter = uniform_set_f4; break;
        case GFX_INT1: result.setter = uniform_set_i1; break;
        case GFX_INT2: result.setter = uniform_set_i2; break;
        case GFX_INT3: result.setter = uniform_set_i3; break;
        case GFX_INT4: result.setter = uniform_set_i4; break;
        case GFX_MAT2: result.setter = uniform_set_m2; break;
        case GFX_MAT3: result.setter = uniform_set_m3; break;
        case GFX_MAT4: result.setter = uniform_set_m4; break;
        default: result.enabled = false; LOG_ERROR("Uniform register failed. Not implemented uniform type: %s\n", name);
    }
    return result;
}

void gfx_uniform_set(gfx_uniform uniform, void *buffer) {
    if (uniform.enabled) {
        uniform.setter(uniform.id, buffer);
    }
}

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
    assert(0 == 1 && "Invalid blend type");
    return GL_ZERO;
}

void gfx_blend( gfx_blend_type src, gfx_blend_type dest) {
    glBlendFunc(gfx_blend_type_to_gl(src), gfx_blend_type_to_gl(dest));
}

void gfx_blend_enable(bool state) {
    if(state) glEnable(GL_BLEND);
    else glDisable(GL_BLEND);
}

gfx_framebuffer_handle gfx_framebuffer_create(struct gfx_framebuffer_desc *framebuffer_desc) {

    gfx_framebuffer_handle handle = OS_MALLOC(sizeof(struct gfx_framebuffer));

    glGenFramebuffers(1, &(handle->id));
    glBindFramebuffer(GL_FRAMEBUFFER, handle->id);


    int32_t color_attachments = 0;
    for(int32_t i=0; i<MAXIMUM_FRAMEBUFFER_COLOR_ATTACHMENTS; ++i)
    {
        gfx_framebuffer_attachment* color_attachment = framebuffer_desc->color_attachments + i;
        if(color_attachment->enabled)
        {
            //todo: assert texture type
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments, GL_TEXTURE_2D, color_attachment->texture_handle->id, 0);
            color_attachments++;
        }
    }

    if(color_attachments == 0)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if(framebuffer_desc->depth_stencil_attachment.enabled)
    {
        gfx_framebuffer_attachment* depth_stencil = &framebuffer_desc->depth_stencil_attachment;
        switch (depth_stencil->texture_handle->type) {

            case GFX_TEXTURE_TYPE_DEPTH:
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                       depth_stencil->texture_handle->id, 0);
                break;
            case GFX_TEXTURE_TYPE_STENCIL:
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                       depth_stencil->texture_handle->id, 0);
                break;
            case GFX_TEXTURE_TYPE_DEPTH_STENCIL:
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                       depth_stencil->texture_handle->id, 0);
                break;
            default: LOG_ERROR("Invalid image type passed as depth stencil attachment\n");
        }
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Could not create framebuffer\n");
    else LOG_INFO("Framebuffer created\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return handle;
}

gfx_texture_type gfx_texture_color_type_from_channels(int32_t channels, bool srgb) {
    switch (channels) {
        case 3:
            if(srgb) return GFX_TEXTURE_TYPE_SRGB;
            return GFX_TEXTURE_TYPE_RGB;
        case 4:
            if(srgb) return GFX_TEXTURE_TYPE_SRGBA;
            return GFX_TEXTURE_TYPE_RGBA;
        default: LOG_ERROR("Could not create texture type, number of channels: %i", channels);
    }
    return GFX_TEXTURE_TYPE_RGB;
}

void gfx_framebuffer_destroy(gfx_framebuffer_handle buffer) {
    glDeleteFramebuffers(1, &buffer->id);
    LOG_INFO("Framebuffer destroyed\n");
    OS_FREE(buffer);
}

void gfx_shader_bind(gfx_shader_handle shader) {
    glUseProgram(shader->id);
}

void gfx_reset_draw_call_count() {
    total_draw_calls = 0;
}

void gfx_viewport_set(int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
}

gfx_texture_handle gfx_texture_create_color(const gfx_texture_desc *desc, gfx_color color) {

    int32_t num_dims = -1;
    switch (desc->type) {

        case GFX_TEXTURE_TYPE_SRGB: num_dims = 3; break;
        case GFX_TEXTURE_TYPE_SRGBA: num_dims = 4; break;
        case GFX_TEXTURE_TYPE_RGB: num_dims = 3; break;
        case GFX_TEXTURE_TYPE_RGBA: num_dims = 4; break;
        case GFX_TEXTURE_TYPE_DEPTH:break;
        case GFX_TEXTURE_TYPE_STENCIL:break;
        case GFX_TEXTURE_TYPE_DEPTH_STENCIL:break;
    }
    if(num_dims == -1)
    {
        LOG_ERROR("Invalid texture format\n");
        return 0;
    }

    for (int32_t i = 0; i < desc->width * desc->height; ++i) {
        for (int32_t j = 0; j < num_dims; ++j) {
            *(((unsigned char *)desc->data) + i + (desc->width * desc->height) * j) = (unsigned char) (
                    *((float *) (&color) + j) * 255);
        }
    }

    return gfx_texture_create(desc);

}
