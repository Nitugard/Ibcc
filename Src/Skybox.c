/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Skybox.h"
#include "Graphics.h"
#include "Allocator.h"
#include "GlMath.h"
#include "Shaders/Common.h"
#include "Device.h"

#ifndef CORE_ASSERT
#include "assert.h"

#define CORE_ASSERT(e) assert(e)
#endif

typedef struct skybox_data {
    gfx_buffer_handle buffer;
    gfx_pipeline_handle pip;
    gfx_texture_cubemap_handle cubemap_texture;
    int32_t view_uniform;
    int32_t projection_uniform;
    gfx_shader_handle shader;
} skybox_data;

float box_vertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
         -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
};

skybox_handle skybox_load(const char* path) {
    skybox_handle handle = OS_MALLOC(sizeof(struct skybox_data));

    const char *names[6] = {
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg"
    };
    handle->shader = gfx_shader_create("Skybox");
    handle->cubemap_texture = gfx_texture_cubemap_create(path, names, GFX_TEXTURE_TYPE_RGB, GFX_TEXTURE_FILTER_LINEAR);
    handle->buffer = gfx_buffer_create(GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW, box_vertices,
                                       sizeof(box_vertices));
    void* fs = device_file_read_text("./Shaders/Skybox.fs");
    void* vs = device_file_read_text("./Shaders/Skybox.vs");
    gfx_shader_add_vs(handle->shader, vs);
    gfx_shader_add_fs(handle->shader, fs);
    OS_FREE(vs);
    OS_FREE(fs);
    gfx_shader_submit(handle->shader);

    gfx_shader_uniform_enable(handle->shader, VIEW_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &handle->view_uniform);
    gfx_shader_uniform_enable(handle->shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &handle->projection_uniform);

    handle->pip = gfx_pipeline_create(handle->shader);
    gfx_pipeline_attr_enable(handle->pip, ATTR_POSITION_NAME, handle->buffer, 3, 0,
                             sizeof(float) * 3);
    gfx_pipeline_submit(handle->pip);
    return handle;
}

void skybox_bind(skybox_handle handle) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_texture_cubemap_bind(handle->cubemap_texture);
}

void skybox_render(skybox_handle handle, float projection[16], float view[16]) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_pipeline_bind(handle->pip);
    gfx_texture_cubemap_bind(handle->cubemap_texture);
    gfx_shader_uniform_set(handle->shader, handle->view_uniform, view);
    gfx_shader_uniform_set(handle->shader, handle->projection_uniform, projection);
    gfx_draw(GFX_TRIANGLES, 0, 36);
}

void skybox_destroy(skybox_handle handle) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_texture_cubemap_destroy(handle->cubemap_texture);
    gfx_buffer_destroy(handle->buffer);
    gfx_pipeline_destroy(handle->pip);
    gfx_shader_destroy(handle->shader);
    OS_FREE(handle);
}
