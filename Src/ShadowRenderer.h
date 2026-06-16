/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef SHADOW_RENDERER_H
#define SHADOW_RENDERER_H

#include <stdint.h>
#include "Graphics.h"
#include "GlMath.h"

#define SHADOW_MAP_SIZE 2048

typedef struct {
    gfx_shader_handle      shader;
    gfx_texture_handle     depth_tex;
    gfx_framebuffer_handle fbo;
    gl_mat                 light_space;
    int32_t                model_uniform;
    int32_t                ls_uniform;
} shadow_renderer;

/* Allocate GPU resources, build light-space matrix. */
void shadow_renderer_init(shadow_renderer* sr);

/* Free all GPU resources. */
void shadow_renderer_destroy(shadow_renderer* sr);

#endif /* SHADOW_RENDERER_H */
