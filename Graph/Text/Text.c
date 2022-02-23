/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Text.h"
#include <Graphics/Graphics.h>
#include <Os/Allocator.h>
#include <Os/Log.h>
#include <Font/Font.h>
#include <Shaders/FontShader.h>
#include <stddef.h>
#include <string.h>
#include <Math/GlMath.h>
#include <Device/Device.h>
#include <Containers/Array.h>

typedef struct text_data{
    void* vertex_buffer;
    gfx_buffer_handle buffer;
    gfx_pipeline_handle pipeline;
    int32_t capacity;
    int32_t current;
    float width;
    float height;
    float scale;
    float offset_x;
    float offset_y;
    gfx_uniform model_uniform;
} text_data;

gfx_shader_handle font_shader_handle;
gfx_texture_handle font_texture_handle;

text_handle text_new(int32_t capacity) {
    text_handle handle = OS_MALLOC(sizeof(struct text_data));

    gfx_buffer_desc buffer_desc = {
            .update_mode = GFX_BUFFER_UPDATE_DYNAMIC_DRAW,
            .size = capacity * sizeof(font_quad),
            .data = OS_MALLOC(capacity * sizeof(struct font_quad)),
            .type = GFX_BUFFER_VERTEX
    };

    handle->vertex_buffer = buffer_desc.data;
    handle->capacity = capacity;
    handle->buffer = gfx_buffer_create(&buffer_desc);
    handle->current = 0;

    gfx_pipeline_desc pipeline_desc = {
            .shader = font_shader_handle,
            .attrs = {
                    [ATTR_POSITION_LOCATION] = {
                            .enabled = true,
                            .buffer = handle->buffer,
                            .stride = sizeof(struct font_vertex),
                            .offset = offsetof(font_vertex, pos),
                            .elements_count = 2,
                            .element_size = 4,
                    },
                    [ATTR_UV_LOCATION] = {
                            .enabled = true,
                            .buffer = handle->buffer,
                            .stride = sizeof(struct font_vertex),
                            .offset = offsetof(font_vertex, uv),
                            .elements_count = 2,
                            .element_size = 4,
                    }
            },
            .name = "font_instance",
    };

    handle->pipeline = gfx_pipeline_create(&pipeline_desc);
    handle->model_uniform = gfx_uniform_register(font_shader_handle, "model", GFX_MAT4);
    return handle;
}

void text_update(text_handle handle, struct text_desc* desc) {
    if (strlen(desc->label) > handle->capacity) {
        LOG_ERROR("Label length exceeds capacity\n");
        return;
    }

    gfx_buffer_desc buffer_desc = {
            .update_mode = GFX_BUFFER_UPDATE_DYNAMIC_DRAW,
            .size = handle->capacity * sizeof(font_quad),
            .data = handle->vertex_buffer,
            .type = GFX_BUFFER_VERTEX
    };
    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    handle->scale = 1 / desc->scale;
    handle->offset_x = desc->offset_x;
    handle->offset_y = desc->offset_y;

    font_result result = font_create(desc->label, handle->vertex_buffer);


    gfx_buffer_update(handle->buffer, &buffer_desc);
    handle->current = strlen(desc->label);
    handle->width = result.width;
    handle->height = result.height;
}

void text_delete(text_handle handle) {
    gfx_pipeline_destroy(handle->pipeline);
    gfx_buffer_destroy(handle->buffer);
    OS_FREE(handle->vertex_buffer);
    OS_FREE(handle);
}

void text_draw(text_handle handle) {

    gfx_blend_enable(true);
    gfx_blend(GFX_BLEND_SRC_COLOR, GFX_BLEND_ONE_MINUS_SRC_COLOR);

    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    //todo: cache and check if width and height have changed to update it
    gl_mat orto = gl_mat_ortographic(-handle->offset_y * handle->scale,
                                     (height - handle->offset_y) * handle->scale,
                                     -handle->offset_x * handle->scale, (width - handle->offset_x) * handle->scale,
                                     -1,
                                     1);

    gfx_pipeline_bind(handle->pipeline);
    gfx_texture_bind(font_texture_handle, 0);
    gfx_uniform_set(handle->model_uniform, orto.data);
    gfx_draw(GFX_TRIANGLES, 0, handle->current * 6);
    gfx_blend_enable(false);
}

void text_init() {
    int32_t width, height, channels;
    font_texture_info(&width, &height, &channels);
    gfx_texture_desc tex_desc = {
            .type = GFX_TEXTURE_TYPE_RGBA,
            .filter = GFX_TEXTURE_FILTER_NEAREST,
            .mipmaps = false,
            .wrap = GFX_TEXTURE_WRAP_CLAMP,
            .width = width,
            .height = height,
            .data = font_texture_get(),
    };
    font_texture_handle = gfx_texture_create(&tex_desc);
    font_shader_handle = gfx_shader_create(&font_shader_desc);
}

void text_terminate() {
    gfx_texture_destroy(font_texture_handle);
    gfx_shader_destroy(font_shader_handle);
}