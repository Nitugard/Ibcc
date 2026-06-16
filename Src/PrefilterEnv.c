/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

/*
 * GPU-based GGX specular pre-filter for IBL split-sum.
 *
 * Follows the exact same render-to-cubemap-face pattern as
 * skybox_create_from_hdr() in Skybox.c, extended with mip levels
 * to encode roughness.
 */

#include "PrefilterEnv.h"
#include "Graphics.h"
#include "Allocator.h"
#include "GlMath.h"
#include "Device.h"
#include "Shaders/Common.h"

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <GL/gl3w.h>
#endif

/*
 * Unit cube — same 36 vertices as box_vertices[] in Skybox.c.
 * Declared static to avoid a link-time collision.
 */
static float s_cube_verts[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

uint32_t prefilter_env_generate(uint32_t env_cubemap_id)
{
    /* ---- Create mipmapped target cubemap ---- */
    uint32_t prefilter_id;
    glGenTextures(1, &prefilter_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_id);

    for (int face = 0; face < 6; ++face) {
        for (int mip = 0; mip < PREFILTER_ENV_MIPS; ++mip) {
            int32_t mip_size = PREFILTER_ENV_BASE_SIZE >> mip;   /* 128, 64, 32, 16, 8 */
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
                         GL_RGB16F, mip_size, mip_size, 0,
                         GL_RGB, GL_FLOAT, 0);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL,  0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL,   PREFILTER_ENV_MIPS - 1);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,  GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,  GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,      GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,      GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,      GL_CLAMP_TO_EDGE);

    /* ---- Build temporary cube pipeline (Skybox.vs + PrefilterEnv.fs) ---- */
    gfx_buffer_handle cube_buf = gfx_buffer_create(
        GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW,
        s_cube_verts, (int32_t)sizeof(s_cube_verts));

    void* vs_src = device_file_read_text("./Shaders/Skybox.vs");
    void* fs_src = device_file_read_text("./Shaders/PrefilterEnv.fs");
    gfx_shader_handle shader = gfx_shader_create("PrefilterEnv");
    gfx_shader_add_vs(shader, vs_src);
    gfx_shader_add_fs(shader, fs_src);
    gfx_shader_submit(shader);
    OS_FREE(vs_src);
    OS_FREE(fs_src);

    int32_t proj_u, view_u, env_u, rough_u;
    gfx_shader_uniform_enable(shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4,   &proj_u);
    gfx_shader_uniform_enable(shader, VIEW_TRANSFORM_NAME,       GFX_TYPE_FLOAT_MAT_4,   &view_u);
    gfx_shader_uniform_enable(shader, "environment",             GFX_TYPE_SAMPLER_CUBE,  &env_u);
    gfx_shader_uniform_enable(shader, "roughness",               GFX_TYPE_FLOAT_VEC_1,   &rough_u);

    gfx_pipeline_handle pipe = gfx_pipeline_create(shader);
    gfx_pipeline_attr_enable(pipe, ATTR_POSITION_NAME, cube_buf, 3, 0, sizeof(float) * 3);
    gfx_pipeline_use_depth_buffer(pipe, false);
    gfx_pipeline_submit(pipe);

    /* ---- 90° perspective + 6 face view matrices (same as Skybox.c) ---- */
    gl_mat capture_proj = gl_mat_perspective(90.0f, 1.0f, 0.1f, 10.0f);
    gl_vec3 eye = gl_vec3_new(0.0f, 0.0f, 0.0f);
    gl_mat capture_views[6] = {
        gl_mat_look_at(gl_vec3_new( 1.0f,  0.0f,  0.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
        gl_mat_look_at(gl_vec3_new(-1.0f,  0.0f,  0.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
        gl_mat_look_at(gl_vec3_new( 0.0f,  1.0f,  0.0f), eye, gl_vec3_new(0.0f,  0.0f,  1.0f)),
        gl_mat_look_at(gl_vec3_new( 0.0f, -1.0f,  0.0f), eye, gl_vec3_new(0.0f,  0.0f, -1.0f)),
        gl_mat_look_at(gl_vec3_new( 0.0f,  0.0f,  1.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
        gl_mat_look_at(gl_vec3_new( 0.0f,  0.0f, -1.0f), eye, gl_vec3_new(0.0f, -1.0f,  0.0f)),
    };

    /* ---- Render all mip levels × all 6 faces ---- */
    uint32_t capture_fbo;
    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    gfx_depth_test_enable(false);
    gfx_cull_enable(false);
    gfx_pipeline_bind(pipe);

    /* Bind source env cubemap to unit 0 */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap_id);
    int32_t env_unit = 0;
    gfx_shader_uniform_set(shader, proj_u, capture_proj.data);
    gfx_shader_uniform_set(shader, env_u,  &env_unit);

    for (int mip = 0; mip < PREFILTER_ENV_MIPS; ++mip) {
        float roughness = (float)mip / (float)(PREFILTER_ENV_MIPS - 1);
        int32_t mip_size = PREFILTER_ENV_BASE_SIZE >> mip;

        glViewport(0, 0, mip_size, mip_size);
        gfx_shader_uniform_set(shader, rough_u, &roughness);

        for (int face = 0; face < 6; ++face) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   prefilter_id, mip);
            gfx_shader_uniform_set(shader, view_u, capture_views[face].data);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &capture_fbo);
    glDepthMask(GL_TRUE);
    gfx_cull_enable(true);
    gfx_depth_test_enable(true);

    /* Release transient resources */
    gfx_pipeline_destroy(pipe);
    gfx_buffer_destroy(cube_buf);
    gfx_shader_destroy(shader);

    return prefilter_id;
}

void prefilter_env_bind(uint32_t id, int32_t unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void prefilter_env_destroy(uint32_t id)
{
    glDeleteTextures(1, &id);
}
