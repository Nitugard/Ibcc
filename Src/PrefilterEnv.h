/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef PREFILTER_ENV_H
#define PREFILTER_ENV_H

#include <stdint.h>

/*
 * Number of roughness levels baked into the prefiltered cubemap.
 * Mip 0 = roughness 0.0 (mirror reflection)
 * Mip 4 = roughness 1.0 (fully blurred, diffuse-like)
 *
 * Each mip halves the base resolution: 128, 64, 32, 16, 8.
 */
#define PREFILTER_ENV_BASE_SIZE  128
#define PREFILTER_ENV_MIPS         5

/*
 * Pre-filter an equirect-converted environment cubemap with GGX specular
 * convolution.  Runs PREFILTER_ENV_MIPS × 6 render passes at startup.
 *
 * env_cubemap_id : raw OpenGL texture ID of the source environment cubemap
 *                 (as returned by gfx_texture_cubemap_get_id)
 *
 * Returns the raw GL texture object ID of the generated mipmapped cubemap.
 * Use prefilter_env_bind() to set it on a texture unit, and
 * prefilter_env_destroy() when the scene is deleted.
 */
uint32_t prefilter_env_generate(uint32_t env_cubemap_id);

/* Bind the prefiltered cubemap to the given GL texture unit (0-based). */
void prefilter_env_bind(uint32_t id, int32_t unit);

/* Delete the GL cubemap texture. */
void prefilter_env_destroy(uint32_t id);

#endif /* PREFILTER_ENV_H */
