/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <GL/gl3w.h>

#include "Graphics.h"

#include <Plugin/Plugin.h>
#include <Log/Log.h>
#include <Os/Allocator.h>

#ifndef GFX_ASSERT
#include <assert.h>
#define GFX_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef struct gfx_shader{
    i32 id;
    gfx_shader_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
} gfx_shader;

typedef struct gfx_buffer{
    u32 id;
} gfx_buffer;

typedef struct gfx_pipeline{
    gfx_shader_handle shader;
    u32 id;
}gfx_pipeline;

plg_desc req_plugins[] = {
                          {.name = "Device", .min_version = 1}};
void plg_on_start(plg_info* info) {

    info->name = "Graphics";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    if (gl3wInit()) {
        LOG_ERROR("Failed to initialize OpenGL\n");
        return false;
    }

    return true;
}


void plg_on_stop(plg_info* info){

}


u32 gl_get_buffer_update_mode(gfx_buffer_update_mode mode)
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
}

u32 gl_get_buffer_type(gfx_buffer_type type) {
    switch (type) {
        case VERTEX:
            return GL_ARRAY_BUFFER;
        case INDEX:
            return GL_ELEMENT_ARRAY_BUFFER;
        default:
            GFX_ASSERT(0 && "Invalid buffer type");
            return 0;
    }
}

void gl_print_shader_err(i32 shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buffer);
    LOG_ERROR("Shader compilation error: \n%s", buffer);
    OS_FREE(buffer);
}

void gl_print_program_err(i32 shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = OS_MALLOC(maxLength);
    glGetProgramInfoLog(shader, maxLength, &maxLength, buffer);
    LOG_ERROR("Program linkage error: \n%s", buffer);
    OS_FREE(buffer);
}

u32 gfx_type_get_size(enum gfx_type type)
{
    switch (type) {
        case GFX_FLOAT: return sizeof(float);
        case GFX_INT: return sizeof(int);
        default:
            GFX_ASSERT(0 && "Gfx invalid type");
            return 0;
    }
}


gfx_shader_handle gfx_shader_create(const gfx_shader_desc *desc) {

    gfx_shader_handle shader = OS_MALLOC(sizeof(gfx_shader));

    shader->id = glCreateProgram();
    memcpy(shader->attrs, desc->attrs, MAXIMUM_PIPELINE_ATTRIBUTES);

    i32 compiled = 0;
    u32 vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vs, GL_SHADER_BINARY_FORMAT_SPIR_V, desc->vs.src, desc->vs.length);
    glSpecializeShader(vs, desc->vs.entry, 0, 0, 0);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, vs);
    else gl_print_shader_err(vs);
    compiled = 0;
    u32 fs = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &fs, GL_SHADER_BINARY_FORMAT_SPIR_V, desc->fs.src, desc->fs.length);
    glSpecializeShader(fs, desc->fs.entry, 0, 0, 0);
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
    return shader;
}

void gfx_shader_destroy(gfx_shader_handle shader) {
    LOG_INFO("Shader destroyed, id: %i\n", shader->id);
    glDeleteProgram(shader->id);
    OS_FREE(shader);
}

gfx_buffer_handle gfx_buffer_create(const gfx_buffer_desc * desc) {
    GFX_ASSERT(desc->size != 0);
    gfx_buffer_handle buffer = OS_MALLOC(sizeof(struct gfx_buffer));

    glGenBuffers(1, &(buffer->id));
    u32 target, update_mode;
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

    typedef struct pip_buff{
        i32 next_offset;
        gfx_buffer_handle buffer;
    } pip_buff;

    bool is_contigous = desc->contiguous_buffer;

    i32 unique_buffers_count = 0;
    pip_buff unique_buffers[MAXIMUM_PIPELINE_ATTRIBUTES];
    i32 offsets[MAXIMUM_PIPELINE_ATTRIBUTES];

    //todo: assert
    gfx_pipeline_handle pipeline = OS_MALLOC(sizeof(struct gfx_buffer));
    pipeline->shader = desc->shader;

    glCreateVertexArrays(1, &(pipeline->id));
    glBindVertexArray(pipeline->id);

    if(is_contigous) {
        memset(unique_buffers, 0, sizeof(unique_buffers));

        for (int i = 0; i < MAXIMUM_PIPELINE_ATTRIBUTES; ++i) {
            gfx_pipeline_attr p_attr = desc->attrs[i];
            if (p_attr.enabled) {
                //todo: check if shader-attr is valid
                gfx_shader_attr s_attr = desc->shader->attrs[i];
                i32 unique_buffer_index = -1;
                for (int j = 0; j < unique_buffers_count; ++j) {
                    if (unique_buffers[j].buffer == p_attr.buffer) {
                        unique_buffer_index = j;
                    }
                }
                if (unique_buffer_index == -1) {
                    unique_buffer_index = unique_buffers_count;
                    unique_buffers_count++;
                }

                pip_buff* p_buff = &unique_buffers[unique_buffer_index];
                offsets[i] = p_buff->next_offset;
                p_buff->buffer = p_attr.buffer;
                p_buff->next_offset += s_attr.num_elements * gfx_type_get_size(s_attr.type);

            }
        }
    }

    i32 unique_buffer_index;
    for(int i=0; i < MAXIMUM_PIPELINE_ATTRIBUTES; ++i) {
        gfx_pipeline_attr p_attr = desc->attrs[i];
        if (p_attr.enabled) {

            //todo:check if shader attr is valid
            gfx_shader_attr s_attr = desc->shader->attrs[i];

            glBindBuffer(GL_ARRAY_BUFFER, p_attr.buffer->id);
            glEnableVertexAttribArray(i);

            if(is_contigous) {
                unique_buffer_index = -1;
                for (int j = 0; j < unique_buffers_count; ++j) {
                    if (unique_buffers[j].buffer == p_attr.buffer) {
                        unique_buffer_index = j;
                    }
                }
                GFX_ASSERT(unique_buffer_index != -1);
                glVertexAttribPointer(i, s_attr.num_elements,
                                      GL_FLOAT, GL_FALSE, unique_buffers[unique_buffer_index].next_offset,
                                      (GLvoid*)(offsets[i]));
            }
            else {

                //todo: type conversion
                glVertexAttribPointer(i, s_attr.num_elements,
                                      GL_FLOAT, GL_FALSE, p_attr.stride,
                                      (GLvoid *) p_attr.offset);
            }
        }
    }

    glBindVertexArray(0);
    return pipeline;
}

void gfx_draw(gfx_pipeline_handle pip) {

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(pip->shader->id);
    glBindVertexArray(pip->id);
    glDrawArrays(GL_TRIANGLES, 0, 3);

}

void gfx_buffer_update(gfx_buffer_handle buffer, const gfx_buffer_desc * desc) {
    glBindBuffer(gl_get_buffer_type(desc->type), buffer->id);
    glBufferSubData(gl_get_buffer_type(desc->type), 0, desc->size, desc->data);
}




