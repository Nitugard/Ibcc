/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <GL/gl3w.h>

#include "Graphics.h"

#include <Os/Log.h>
#include <Os/Allocator.h>

#ifndef GFX_ASSERT
#include <assert.h>
#define GFX_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef struct gfx_shader{
    int32_t id;
    gfx_shader_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
} gfx_shader;

typedef struct gfx_buffer{
    uint32_t id;
} gfx_buffer;

typedef struct gfx_uniform{
    int32_t id;
    void(* gfx_uniform_set)(struct gfx_uniform* uniform);
    void* buffer;
    uint32_t offset;
} gfx_uniform;

typedef struct gfx_pipeline{
    gfx_shader_handle shader;
    uint32_t vao_id;
    uint32_t ebo_id;
    gfx_uniform uniforms[MAXIMUM_PIPELINE_UNIFORMS];
    uint32_t active_uniforms;
}gfx_pipeline;

typedef struct gfx_texture{
    uint32_t id;
    int32_t width;
    int32_t height;
} gfx_texture;

__stdcall void opengl_msg_callback( GLenum source,GLenum type, GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam )
{

    LOG_ERROR("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

/*
 * Uniform settters.
 */

void uniform_set_f1(gfx_uniform* data){glUniform1f(data->id, *(float*)data->buffer);}
void uniform_set_f2(gfx_uniform* data){glUniform2f(data->id, *(float*)data->buffer, *((float*)data->buffer + 1));}
void uniform_set_f3(gfx_uniform* data){glUniform3f(data->id, *(float*)data->buffer, *((float*)data->buffer + 1), *((float*)data->buffer + 2));}
void uniform_set_f4(gfx_uniform* data){glUniform4f(data->id, *(float*)data->buffer, *((float*)data->buffer + 1), *((float*)data->buffer + 2), *((float*)data->buffer + 3));}
void uniform_set_i1(gfx_uniform* data){glUniform1i(data->id, *(int*)data->buffer);}
void uniform_set_i2(gfx_uniform* data){glUniform2i(data->id, *(int*)data->buffer, *((int*)data->buffer + 1));}
void uniform_set_i3(gfx_uniform* data){glUniform3i(data->id, *(int*)data->buffer, *((int*)data->buffer + 1), *((int*)data->buffer + 2));}
void uniform_set_i4(gfx_uniform* data){glUniform4i(data->id, *(int*)data->buffer, *((int*)data->buffer + 1), *((int*)data->buffer + 2), *((int*)data->buffer + 3));}
void uniform_set_mat2(gfx_uniform* data){glUniformMatrix2fv(data->id, 1, 0, (float*)data->buffer);}
void uniform_set_mat3(gfx_uniform* data){glUniformMatrix3fv(data->id, 1, 0, (float*)data->buffer);}
void uniform_set_mat4(gfx_uniform* data){glUniformMatrix4fv(data->id, 1, 0, (float*)(data->buffer));}
void uniform_set_sampler2(gfx_uniform* data){glActiveTexture(data->id); glBindTexture(data->id, *(int*)data->buffer);}
void uniform_set_sampler3(gfx_uniform* data){glActiveTexture(data->id); glBindTexture(data->id, *(int*)data->buffer);}


bool gfx_init() {
    if (gl3wInit()) {
        LOG_ERROR("Failed to initialize OpenGL\n");
        return false;
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_msg_callback, 0);

    return true;
}


void gfx_terminate(){

}


uint32_t gl_get_buffer_update_mode(gfx_buffer_update_mode mode)
{
    switch (mode) {
        case STATIC_DRAW:
            return GL_STATIC_DRAW;
        case STATIC_READ:
            return GL_STATIC_READ;
        case STATIC_COPY:
            return GL_STATIC_COPY;
        case DYNAMIC_DRAW:
            return GL_DYNAMIC_DRAW;
        case DYNAMIC_READ:
            return GL_DYNAMIC_READ;
        case DYNAMIC_COPY:
            return GL_DYNAMIC_COPY;
        case STREAM_DRAW:
            return GL_STREAM_DRAW;
        case STREAM_READ:
            return GL_STREAM_READ;
        case STREAM_COPY:
            return GL_STREAM_COPY;
        default:
            break;
    }
    GFX_ASSERT(0==1);
}

uint32_t gl_get_buffer_type(gfx_buffer_type type) {
    switch (type) {
        case VERTEX:
            return GL_ARRAY_BUFFER;
        case INDEX:
            return GL_ELEMENT_ARRAY_BUFFER;
        case UNIFORM:
            return GL_UNIFORM_BUFFER;
        default:
            GFX_ASSERT(0 && "Invalid buffer type");
            return 0;
    }
}

void gl_print_shader_err(int32_t shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buffer);
    LOG_ERROR("Shader compilation error: \n%s", buffer);
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
    memcpy(shader->attrs, desc->attrs, MAXIMUM_PIPELINE_ATTRIBUTES);

    int32_t compiled = 0;
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &desc->vs.src, 0);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, vs);
    else gl_print_shader_err(vs);
    compiled = 0;
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &desc->fs.src, 0);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, fs);
    else gl_print_shader_err(fs);

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

    if(desc->type == UNIFORM && desc->size >= GL_MAX_UNIFORM_BLOCK_SIZE) {
        LOG_ERROR("Failed to create shader. Uniform block size reached its limit!");
        return 0;
    }

    GFX_ASSERT(desc->size != 0);
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



gfx_pipeline_handle gfx_pipeline_create(const gfx_pipeline_desc *desc) {

    gfx_pipeline_handle pipeline = OS_MALLOC(sizeof(struct gfx_pipeline));
    pipeline->shader = desc->shader;

    glCreateVertexArrays(1, &(pipeline->vao_id));
    glBindVertexArray(pipeline->vao_id);

    if(desc->index_buffer != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, desc->index_buffer->id);
        pipeline->ebo_id = desc->index_buffer->id;
    }
    else pipeline->ebo_id = GL_INVALID_INDEX;

    for(int i=0; i < MAXIMUM_PIPELINE_ATTRIBUTES; ++i) {
        gfx_pipeline_attr p_attr = desc->attrs[i];
        if (p_attr.buffer != 0) {
            GFX_ASSERT(p_attr.stride != 0);
            GFX_ASSERT(p_attr.offset < p_attr.stride);

            //todo:check if shader attr is valid
            gfx_shader_attr s_attr = desc->shader->attrs[i];

            glBindBuffer(GL_ARRAY_BUFFER, p_attr.buffer->id);
            glEnableVertexAttribArray(i);

            glVertexAttribPointer(i, s_attr.num_elements,
                                  GL_FLOAT, GL_FALSE, p_attr.stride,
                                  (GLvoid *) p_attr.offset);
        }
    }
    int32_t uniforms_count;
    glGetProgramiv(desc->shader->id, GL_ACTIVE_UNIFORMS, &uniforms_count);
    int32_t size, length;
    uint32_t type;
    char name[1024];
    int32_t active_uniforms = 0;
    for (int32_t i = 0; i < uniforms_count; i++) {
        glGetActiveUniform(desc->shader->id, (GLuint) i, sizeof(name) / sizeof(char), &length, &size, &type, name);
        for (int j = 0; j < MAXIMUM_PIPELINE_UNIFORMS; ++j) {
            gfx_pipeline_uniform uniform = desc->uniforms[j];

            if (uniform.buffer != 0 && strcmp(uniform.name, name) == 0) {
                struct gfx_uniform* uni = pipeline->uniforms + active_uniforms;
                ++active_uniforms;
                //todo: check if uniform with same index is already acitve
                uni->buffer = uniform.buffer;
                uni->id = glGetUniformLocation(desc->shader->id, name);
                uni->offset = uniform.offset;
                switch (type) {
                    case GL_FLOAT: uni->gfx_uniform_set = uniform_set_f1; break;
                    case GL_FLOAT_VEC2: uni->gfx_uniform_set = uniform_set_f2; break;
                    case GL_FLOAT_VEC3: uni->gfx_uniform_set = uniform_set_f3; break;
                    case GL_FLOAT_VEC4: uni->gfx_uniform_set = uniform_set_f4; break;
                    case GL_INT: uni->gfx_uniform_set = uniform_set_i1; break;
                    case GL_INT_VEC2: uni->gfx_uniform_set = uniform_set_i2; break;
                    case GL_INT_VEC3: uni->gfx_uniform_set = uniform_set_i3; break;
                    case GL_INT_VEC4: uni->gfx_uniform_set = uniform_set_i4; break;
                    case GL_FLOAT_MAT2: uni->gfx_uniform_set = uniform_set_mat2; break;
                    case GL_FLOAT_MAT3: uni->gfx_uniform_set = uniform_set_mat3; break;
                    case GL_FLOAT_MAT4: uni->gfx_uniform_set = uniform_set_mat4; break;
                    default: GFX_ASSERT(false && "Not implemented uniform type");
                }
            }
        }
    }


    int32_t binding_point = 0;
    for(int32_t i=0; i < MAXIMUM_PIPELINE_UNIFORM_BLOCKS; ++i)
    {
        if(binding_point >= GL_MAX_UNIFORM_BUFFER_BINDINGS) {
            LOG_ERROR("Uniform block limit reached %s\n", desc->shader);
            break;
        }

        gfx_pipeline_uniform_block block = desc->uniform_blocks[i];
        if(block.buffer != 0)
        {
            uint32_t block_id = glGetUniformBlockIndex(desc->shader->id, block.name);
            if(GL_INVALID_INDEX == block_id) {
                LOG_ERROR("Uniform block with name %s has not been found for shader %s\n", block.name,
                          desc->shader->id);
                continue;
            }
            glUniformBlockBinding(desc->shader->id, block_id, binding_point);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, block.buffer->id);
            ++binding_point;
        }
    }

    pipeline->active_uniforms = active_uniforms;
    glBindVertexArray(0);
    return pipeline;
}

void gfx_apply_pipeline(gfx_pipeline_handle pip) {

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(pip->shader->id);
    glBindVertexArray(pip->vao_id);
    if(pip->ebo_id != GL_INVALID_INDEX)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pip->ebo_id);

}

void gfx_update_uniforms(gfx_pipeline_handle pip, uint32_t start, int32_t length) {
    glUseProgram(pip->shader->id);
    if(length == -1) length = pip->active_uniforms;
    for (uint32_t i = start; i < length; ++i) {
        struct gfx_uniform uni = pip->uniforms[i];
        uni.buffer = (char*)uni.buffer + uni.offset;
        uni.gfx_uniform_set(&uni);
    }
}

void gfx_draw_triangles(int32_t start, int32_t length)
{
    glDrawArrays(GL_TRIANGLES, 0, length);
}

void gfx_draw_lines(int32_t start, int32_t length) {
    glDrawArrays(GL_LINES, 0, length);
}

void gfx_draw_triangles_indexed(int32_t length)
{
    glDrawElements(GL_TRIANGLES, length, GL_UNSIGNED_INT, 0);
}

void gfx_buffer_update(gfx_buffer_handle buffer, const gfx_buffer_desc * desc) {
    glBindBuffer(gl_get_buffer_type(desc->type), buffer->id);
    glBufferSubData(gl_get_buffer_type(desc->type), 0, desc->size, desc->data);
}

void gfx_pipeline_destroy(gfx_pipeline_handle hndl) {
    //TODO
    glDeleteVertexArrays(1, &(hndl->vao_id));
    OS_FREE(hndl);
}

void gfx_begin_default_pass(const gfx_pass_action * action) {
    switch (action->action) {
        case GFX_ACTION_CLEAR:
        glClearColor(action->value.x, action->value.y, action->value.z, action->value.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
    }
}

void gfx_end_pass() {

}

gfx_texture_handle gfx_texture_create(const gfx_texture_desc * desc) {

    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc->width, desc->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, desc->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    gfx_texture_handle hndl = OS_MALLOC(sizeof(gfx_texture));
    hndl->width = desc->width;
    hndl->height = desc->height;
    hndl->id = texture;
    return hndl;
}

void gfx_texture_bind(gfx_texture_handle hndl) {

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hndl->id);
}

void gfx_texture_destroy(gfx_texture_handle hndl) {
    glDeleteTextures(1, &(hndl->id));
    OS_FREE(hndl);
}