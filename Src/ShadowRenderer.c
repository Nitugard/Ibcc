/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "ShadowRenderer.h"
#include "Allocator.h"
#include "Device.h"
#include "GlMath.h"
#include "Shaders/Common.h"

void shadow_renderer_init(shadow_renderer* sr)
{
    /* Depth-only shader */
    sr->shader = gfx_shader_create("Shadow");
    void* vs = device_file_read_text("./Shaders/Shadow.vs");
    void* fs = device_file_read_text("./Shaders/Shadow.fs");
    gfx_shader_add_vs(sr->shader, vs);
    gfx_shader_add_fs(sr->shader, fs);
    gfx_shader_submit(sr->shader);
    OS_FREE(vs);
    OS_FREE(fs);

    gfx_shader_uniform_enable(sr->shader, MODEL_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &sr->model_uniform);
    gfx_shader_uniform_enable(sr->shader, "light_space",        GFX_TYPE_FLOAT_MAT_4, &sr->ls_uniform);

    /* Depth texture + depth-only FBO */
    sr->depth_tex = gfx_texture_create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, NULL,
                                       GFX_TEXTURE_TYPE_DEPTH,
                                       GFX_TEXTURE_FILTER_NEAREST,
                                       GFX_TEXTURE_WRAP_CLAMP);
    sr->fbo = gfx_framebuffer_create(NULL, sr->depth_tex);

    /*
     * Orthographic light from sun direction normalize(1,2,1).
     * gl_mat_look_at(forward_dir, eye, up) — first arg is direction, not target.
     */
    gl_vec3 sun_dir   = gl_vec3_normalize(gl_vec3_new(1.0f, 2.0f, 1.0f));
    gl_vec3 light_pos = gl_vec3_new(sun_dir.x * 15.0f, sun_dir.y * 15.0f, sun_dir.z * 15.0f);
    gl_vec3 target    = gl_vec3_new(0.0f, 2.0f, 0.0f);
    gl_vec3 up        = gl_vec3_new(1.0f, 0.0f, 0.0f);   /* avoid Y-parallel with sun */
    gl_vec3 forward   = gl_vec3_normalize(gl_vec3_sub(target, light_pos));
    gl_mat  lv        = gl_mat_look_at(forward, light_pos, up);
    gl_mat  lp        = gl_mat_ortographic(-8.0f, 8.0f, -8.0f, 8.0f, 0.5f, 50.0f);
    sr->light_space   = gl_mat_mul(lp, lv);
}

void shadow_renderer_destroy(shadow_renderer* sr)
{
    gfx_shader_destroy(sr->shader);
    gfx_texture_destroy(sr->depth_tex);
    gfx_framebuffer_destroy(sr->fbo);
}
