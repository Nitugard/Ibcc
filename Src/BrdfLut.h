/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef BRDF_LUT_H
#define BRDF_LUT_H

#include "Graphics.h"

/*
 * Generate a 512×512 BRDF integration LUT using a single GPU render pass.
 *
 *   R channel : scale  — Σ f(l,v)·NdotL·(1-(1-VdotH)^5) / N
 *   G channel : bias   — Σ f(l,v)·NdotL·(1-VdotH)^5     / N
 *
 * X axis = NdotV (0 → 1), Y axis = roughness (0 → 1).
 *
 * Caller owns the returned texture and must call gfx_texture_destroy() on it.
 */
gfx_texture_handle brdf_lut_generate(void);

#endif /* BRDF_LUT_H */
