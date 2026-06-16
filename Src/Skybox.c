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

#include <string.h>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <GL/gl3w.h>
#endif

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
    int32_t skybox_uniform;
    int32_t exposure_uniform;
    float exposure;
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

static bool skybox_path_is_hdr(const char* path)
{
    const char* ext = strrchr(path, '.');
    return ext != 0 && (strcmp(ext, ".hdr") == 0 || strcmp(ext, ".HDR") == 0);
}

static gfx_texture_cubemap_handle skybox_create_from_hdr(const char* path, gfx_buffer_handle cube_buffer)
{
    const int32_t capture_size = 512;
    gfx_texture_handle equirectangular_map = gfx_texture_load_hdr(path, GFX_TEXTURE_FILTER_LINEAR, GFX_TEXTURE_WRAP_CLAMP);
    gfx_texture_cubemap_handle env_cubemap = gfx_texture_cubemap_create_empty_hdr(capture_size);

    gfx_shader_handle shader = gfx_shader_create("EquirectToCube");
    void* fs = device_file_read_text("./Shaders/EquirectToCube.fs");
    void* vs = device_file_read_text("./Shaders/Skybox.vs");
    gfx_shader_add_vs(shader, vs);
    gfx_shader_add_fs(shader, fs);
    OS_FREE(vs);
    OS_FREE(fs);
    gfx_shader_submit(shader);

    int32_t projection_uniform;
    int32_t view_uniform;
    int32_t equirectangular_uniform;
    gfx_shader_uniform_enable(shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &projection_uniform);
    gfx_shader_uniform_enable(shader, VIEW_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &view_uniform);
    gfx_shader_uniform_enable(shader, "equirectangular_map", GFX_TYPE_SAMPLER_2D, &equirectangular_uniform);

    gfx_pipeline_handle pipeline = gfx_pipeline_create(shader);
    gfx_pipeline_attr_enable(pipeline, ATTR_POSITION_NAME, cube_buffer, 3, 0, sizeof(float) * 3);
    gfx_pipeline_use_depth_buffer(pipeline, false);
    gfx_pipeline_submit(pipeline);

    uint32_t capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glViewport(0, 0, capture_size, capture_size);

    gl_mat capture_projection = gl_mat_perspective(90.0f, 1.0f, 0.1f, 10.0f);
    gl_vec3 eye = gl_vec3_new(0.0f, 0.0f, 0.0f);
    gl_mat capture_views[6] = {
            gl_mat_look_at(gl_vec3_new( 1.0f,  0.0f,  0.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
            gl_mat_look_at(gl_vec3_new(-1.0f,  0.0f,  0.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
            gl_mat_look_at(gl_vec3_new( 0.0f,  1.0f,  0.0f), eye, gl_vec3_new(0.0f,  0.0f,  1.0f)),
            gl_mat_look_at(gl_vec3_new( 0.0f, -1.0f,  0.0f), eye, gl_vec3_new(0.0f,  0.0f, -1.0f)),
            gl_mat_look_at(gl_vec3_new( 0.0f,  0.0f,  1.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
            gl_mat_look_at(gl_vec3_new( 0.0f,  0.0f, -1.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
    };

    gfx_depth_test_enable(false);
    gfx_cull_enable(false);
    gfx_pipeline_bind(pipeline);
    gfx_texture_bind(equirectangular_map, 0);
    int32_t texture_unit = 0;
    gfx_shader_uniform_set(shader, projection_uniform, capture_projection.data);
    gfx_shader_uniform_set(shader, equirectangular_uniform, &texture_unit);

    for (uint32_t i = 0; i < 6; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               gfx_texture_cubemap_get_id(env_cubemap),
                               0);
        CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE &&
                    "HDR cubemap conversion framebuffer incomplete");

        gfx_shader_uniform_set(shader, view_uniform, capture_views[i].data);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);
    glDepthMask(GL_TRUE);
    gfx_cull_enable(true);
    gfx_depth_test_enable(true);

    gfx_pipeline_destroy(pipeline);
    gfx_shader_destroy(shader);
    gfx_texture_destroy(equirectangular_map);
    return env_cubemap;
}

skybox_handle skybox_load(const char* path) {
    skybox_handle handle = OS_MALLOC(sizeof(struct skybox_data));

    handle->shader = gfx_shader_create("Skybox");
    handle->exposure = 1.0f;
    handle->buffer = gfx_buffer_create(GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW, box_vertices,
                                       sizeof(box_vertices));

    if (skybox_path_is_hdr(path)) {
        handle->cubemap_texture = skybox_create_from_hdr(path, handle->buffer);
    }
    else {
        const char *names[6] = {
                "px.png",
                "nx.png",
                "py.png",
                "ny.png",
                "pz.png",
                "nz.png"
        };
        handle->cubemap_texture = gfx_texture_cubemap_create(path, names, GFX_TEXTURE_TYPE_RGB, GFX_TEXTURE_FILTER_LINEAR);
    }

    void* fs = device_file_read_text("./Shaders/Skybox.fs");
    void* vs = device_file_read_text("./Shaders/Skybox.vs");
    gfx_shader_add_vs(handle->shader, vs);
    gfx_shader_add_fs(handle->shader, fs);
    OS_FREE(vs);
    OS_FREE(fs);
    gfx_shader_submit(handle->shader);

    gfx_shader_uniform_enable(handle->shader, VIEW_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &handle->view_uniform);
    gfx_shader_uniform_enable(handle->shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &handle->projection_uniform);
    gfx_shader_uniform_enable(handle->shader, "skybox", GFX_TYPE_SAMPLER_CUBE, &handle->skybox_uniform);
    gfx_shader_uniform_enable(handle->shader, "exposure", GFX_TYPE_FLOAT_VEC_1, &handle->exposure_uniform);

    handle->pip = gfx_pipeline_create(handle->shader);
    gfx_pipeline_attr_enable(handle->pip, ATTR_POSITION_NAME, handle->buffer, 3, 0,
                             sizeof(float) * 3);
    gfx_pipeline_use_depth_buffer(handle->pip, false);
    gfx_pipeline_submit(handle->pip);
    return handle;
}

void skybox_bind(skybox_handle handle) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_texture_cubemap_bind(handle->cubemap_texture);
}

void skybox_render(skybox_handle handle, float projection[16], float view[16]) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_depth_test_enable(false);
    gfx_cull_enable(false);
    gfx_pipeline_bind(handle->pip);
    gfx_texture_cubemap_bind(handle->cubemap_texture);
    gfx_shader_uniform_set(handle->shader, handle->view_uniform, view);
    gfx_shader_uniform_set(handle->shader, handle->projection_uniform, projection);
    int32_t texture_unit = 0;
    gfx_shader_uniform_set(handle->shader, handle->skybox_uniform, &texture_unit);
    gfx_shader_uniform_set(handle->shader, handle->exposure_uniform, &handle->exposure);
    gfx_draw(GFX_TRIANGLES, 0, 36);
    gfx_cull_enable(true);
    gfx_depth_test_enable(true);
}

float skybox_get_exposure(skybox_handle handle) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    return handle->exposure;
}

void skybox_set_exposure(skybox_handle handle, float exposure) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    handle->exposure = exposure;
}

void skybox_destroy(skybox_handle handle) {
    CORE_ASSERT(handle != 0 && "Invalid skybox pointer");
    gfx_texture_cubemap_destroy(handle->cubemap_texture);
    gfx_buffer_destroy(handle->buffer);
    gfx_pipeline_destroy(handle->pip);
    gfx_shader_destroy(handle->shader);
    OS_FREE(handle);
}
