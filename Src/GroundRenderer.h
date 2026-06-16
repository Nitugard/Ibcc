/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef GROUND_RENDERER_H
#define GROUND_RENDERER_H

#include <stdint.h>
#include "Graphics.h"

typedef struct {
    gfx_shader_handle   shader;
    gfx_pipeline_handle pipeline;
    gfx_buffer_handle   vbuf;
    gfx_buffer_handle   ibuf;

    int32_t model_u, proj_u, view_u, viewpos_u;
    int32_t roughness_u, metallic_u, color_u;
    int32_t skybox_u, exposure_u, shadowmap_u, lightspace_u, brdf_lut_u;
} ground_renderer;

/* Allocate GPU resources (20×20 quad + Lit shader). */
void ground_renderer_init(ground_renderer* gr);

/* Draw the ground plane with current view/light state. */
void ground_renderer_render(ground_renderer* gr,
                             float projection[16],
                             float view[16],
                             float view_pos[3],
                             int32_t skybox_unit,
                             float   exposure,
                             int32_t shadow_unit,
                             float   light_space[16],
                             int32_t brdf_unit);

/* Free all GPU resources. */
void ground_renderer_destroy(ground_renderer* gr);

#endif /* GROUND_RENDERER_H */
