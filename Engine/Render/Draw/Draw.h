/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_DRAW_H
#define IBC_DRAW_H

#include <Graphics/Graphics.h>
#include "Math/Math.h"

#ifndef API
#define API
#endif

#define MAXIMUM_VERTICES 1024

typedef struct dw_desc{
    gfx_buffer_handle cam_buffer;
} dw_desc;

typedef struct dw_data* dw_handle;

API dw_handle dw_new(dw_desc const* desc);
API void dw_begin(dw_handle handle);//todo: in case of static drawing
API void dw_vector(dw_handle handle, mm_vec3 vec, mm_vec3 color);
API void dw_vector_origin(dw_handle handle, mm_vec3 vec, mm_vec3 origin, mm_vec3 color);
API void dw_end(dw_handle handle);

API void dw_bind(dw_handle handle);

#endif //IBC_DRAW_H
