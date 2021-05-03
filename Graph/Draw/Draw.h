/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_DRAW_H
#define IBC_DRAW_H

#include <Graphics/Graphics.h>
#include "Math/GlMath.h"

#ifndef API
#define API
#endif


typedef struct dw_desc{
} dw_desc;

typedef struct dw_data* dw_handle;

API dw_handle dw_new(dw_desc const* desc, int32_t capacity);

API void dw_segment_origin(dw_handle handle, gl_vec3 vec, gl_vec3 color);
API void dw_segment(dw_handle handle, gl_vec3 vec, gl_vec3 origin, gl_vec3 color);
API void dw_grid(dw_handle handle, uint32_t segments);
API void dw_cube(dw_handle handle, gl_vec3 pos, gl_vec3 color, gl_vec3 extents);

API void dw_clear(dw_handle handle, uint64_t position);
API void dw_clear_all(dw_handle handle);

API uint64_t dw_get_position(dw_handle handle);
API void dw_draw(dw_handle handle, float projection[16], float view[16]);

API void dw_apply(dw_handle handle);
API void dw_delete(dw_handle handle);

#endif //IBC_DRAW_H
