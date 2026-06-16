/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "BrdfLut.h"
#include "Allocator.h"
#include "Device.h"

#define BRDF_LUT_SIZE 512

gfx_texture_handle brdf_lut_generate(void)
{
    /*
     * Target texture: RGB16 gives enough precision for the [0,1] scale/bias
     * values without clamping artifacts.  We only use RG but RGB16 is the
     * smallest renderable float format available in this engine.
     */
    gfx_texture_handle lut = gfx_texture_create(
        BRDF_LUT_SIZE, BRDF_LUT_SIZE, NULL,
        GFX_TEXTURE_TYPE_RGB16,
        GFX_TEXTURE_FILTER_LINEAR,
        GFX_TEXTURE_WRAP_CLAMP);

    gfx_framebuffer_handle fbo = gfx_framebuffer_create(lut, NULL);

    /* Load integration shaders */
    void* vs_src = device_file_read_text("./Shaders/BrdfLut.vs");
    void* fs_src = device_file_read_text("./Shaders/BrdfLut.fs");
    gfx_shader_handle shader = gfx_shader_create("BrdfLut");
    gfx_shader_add_vs(shader, vs_src);
    gfx_shader_add_fs(shader, fs_src);
    gfx_shader_submit(shader);
    OS_FREE(vs_src);
    OS_FREE(fs_src);

    /* Fullscreen quad covering [-1,1]×[-1,1] */
    float verts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };
    uint32_t idx[] = {0, 1, 2,  0, 2, 3};

    gfx_buffer_handle vbuf = gfx_buffer_create(
        GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW,
        verts, (int32_t)sizeof(verts));
    gfx_buffer_handle ibuf = gfx_buffer_create(
        GFX_BUFFER_INDEX, GFX_BUFFER_UPDATE_STATIC_DRAW,
        idx, (int32_t)sizeof(idx));

    gfx_pipeline_handle pipe = gfx_pipeline_create(shader);
    gfx_pipeline_attr_enable(pipe, "pos", vbuf, 2, 0, 2 * (int32_t)sizeof(float));
    gfx_pipeline_index_enable(pipe, ibuf);
    gfx_pipeline_submit(pipe);

    /* Single render pass — no depth test, no culling */
    static float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    gfx_begin_pass(fbo, GFX_PASS_OPTION_NONE,
                   GFX_PASS_ACTION_CLEAR_COLOR, black);
    gfx_viewport_set(BRDF_LUT_SIZE, BRDF_LUT_SIZE);
    gfx_pipeline_bind(pipe);
    gfx_draw_id(GFX_TRIANGLES, 6);
    gfx_end_pass();

    /* Transient resources no longer needed */
    gfx_pipeline_destroy(pipe);
    gfx_buffer_destroy(vbuf);
    gfx_buffer_destroy(ibuf);
    gfx_shader_destroy(shader);
    gfx_framebuffer_destroy(fbo);

    return lut;
}
