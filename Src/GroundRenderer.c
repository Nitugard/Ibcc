/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "GroundRenderer.h"
#include "Allocator.h"
#include "Device.h"
#include "GlMath.h"
#include "Shaders/Common.h"

void ground_renderer_init(ground_renderer* gr)
{
    /*
     * 20×20 quad at y=0.
     * Vertex layout: pos(3) normal(3) tangent(4) color(4) = 14 floats, stride 56 bytes.
     * Winding {0,2,1, 0,3,2} → CCW from above → +Y normal, passes back-face cull.
     */
    float verts[] = {
        -10.0f, 0.0f, -10.0f,  0.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,
         10.0f, 0.0f, -10.0f,  0.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,
         10.0f, 0.0f,  10.0f,  0.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,
        -10.0f, 0.0f,  10.0f,  0.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,
    };
    uint32_t idx[] = {0, 2, 1,  0, 3, 2};
    int stride = 14 * (int)sizeof(float);

    gr->vbuf = gfx_buffer_create(GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW, verts, sizeof(verts));
    gr->ibuf = gfx_buffer_create(GFX_BUFFER_INDEX,  GFX_BUFFER_UPDATE_STATIC_DRAW, idx,   sizeof(idx));

    void* vs = device_file_read_text("./Shaders/Lit.vs");
    void* fs = device_file_read_text("./Shaders/Lit.fs");
    gr->shader = gfx_shader_create("Ground");
    gfx_shader_add_vs(gr->shader, vs);
    gfx_shader_add_fs(gr->shader, fs);
    gfx_shader_submit(gr->shader);
    OS_FREE(vs);
    OS_FREE(fs);

    gr->pipeline = gfx_pipeline_create(gr->shader);
    gfx_pipeline_attr_enable(gr->pipeline, ATTR_POSITION_NAME, gr->vbuf, 3,  0,                stride);
    gfx_pipeline_attr_enable(gr->pipeline, ATTR_NORMAL_NAME,   gr->vbuf, 3,  3*sizeof(float),  stride);
    gfx_pipeline_attr_enable(gr->pipeline, ATTR_TANGENT_NAME,  gr->vbuf, 4,  6*sizeof(float),  stride);
    gfx_pipeline_attr_enable(gr->pipeline, ATTR_COLOR_NAME,    gr->vbuf, 4, 10*sizeof(float),  stride);
    gfx_pipeline_index_enable(gr->pipeline, gr->ibuf);
    gfx_pipeline_submit(gr->pipeline);

    gfx_shader_uniform_enable(gr->shader, MODEL_TRANSFORM_NAME,      GFX_TYPE_FLOAT_MAT_4, &gr->model_u);
    gfx_shader_uniform_enable(gr->shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &gr->proj_u);
    gfx_shader_uniform_enable(gr->shader, VIEW_TRANSFORM_NAME,       GFX_TYPE_FLOAT_MAT_4, &gr->view_u);
    gfx_shader_uniform_enable(gr->shader, VIEW_POSITION_NAME,        GFX_TYPE_FLOAT_VEC_3, &gr->viewpos_u);
    gfx_shader_uniform_enable(gr->shader, "roughness",               GFX_TYPE_FLOAT_VEC_1, &gr->roughness_u);
    gfx_shader_uniform_enable(gr->shader, "metallic",                GFX_TYPE_FLOAT_VEC_1, &gr->metallic_u);
    gfx_shader_uniform_enable(gr->shader, "base_color",              GFX_TYPE_FLOAT_VEC_3, &gr->color_u);
    gfx_shader_uniform_enable(gr->shader, "skybox",                  GFX_TYPE_SAMPLER_CUBE, &gr->skybox_u);
    gfx_shader_uniform_enable(gr->shader, "exposure",                GFX_TYPE_FLOAT_VEC_1, &gr->exposure_u);
    gfx_shader_uniform_enable(gr->shader, "shadow_map",              GFX_TYPE_SAMPLER_2D,  &gr->shadowmap_u);
    gfx_shader_uniform_enable(gr->shader, "light_space",             GFX_TYPE_FLOAT_MAT_4, &gr->lightspace_u);
    gfx_shader_uniform_enable(gr->shader, "brdf_lut",                GFX_TYPE_SAMPLER_2D,  &gr->brdf_lut_u);
    gfx_shader_uniform_enable(gr->shader, "prefiltered_env",         GFX_TYPE_SAMPLER_CUBE, &gr->prefiltered_env_u);
}

void ground_renderer_render(ground_renderer* gr,
                             float projection[16], float view[16], float view_pos[3],
                             int32_t skybox_unit, float exposure,
                             int32_t shadow_unit, float light_space[16],
                             int32_t brdf_unit, int32_t prefilter_unit)
{
    static const float color[3]    = {0.50f, 0.50f, 0.48f};
    static       float roughness   = 0.92f;
    static       float metallic    = 0.0f;
    gl_mat model = gl_mat_new_identity();

    gfx_pipeline_bind(gr->pipeline);
    gfx_shader_uniform_set(gr->shader, gr->model_u,      model.data);
    gfx_shader_uniform_set(gr->shader, gr->proj_u,       projection);
    gfx_shader_uniform_set(gr->shader, gr->view_u,       view);
    gfx_shader_uniform_set(gr->shader, gr->viewpos_u,    view_pos);
    gfx_shader_uniform_set(gr->shader, gr->roughness_u,  &roughness);
    gfx_shader_uniform_set(gr->shader, gr->metallic_u,   &metallic);
    gfx_shader_uniform_set(gr->shader, gr->color_u,      (float*)color);
    gfx_shader_uniform_set(gr->shader, gr->skybox_u,     &skybox_unit);
    gfx_shader_uniform_set(gr->shader, gr->exposure_u,   &exposure);
    gfx_shader_uniform_set(gr->shader, gr->shadowmap_u,  &shadow_unit);
    gfx_shader_uniform_set(gr->shader, gr->lightspace_u, light_space);
    gfx_shader_uniform_set(gr->shader, gr->brdf_lut_u,          &brdf_unit);
    gfx_shader_uniform_set(gr->shader, gr->prefiltered_env_u,   &prefilter_unit);
    gfx_draw_id(GFX_TRIANGLES, 6);
}

void ground_renderer_destroy(ground_renderer* gr)
{
    gfx_pipeline_destroy(gr->pipeline);
    gfx_buffer_destroy(gr->vbuf);
    gfx_buffer_destroy(gr->ibuf);
    gfx_shader_destroy(gr->shader);
}
