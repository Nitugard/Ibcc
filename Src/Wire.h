/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_WIRE_H
#define IBCWEB_WIRE_H

#ifndef IBC_API
#define IBC_API extern
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct wire_data* wire_handle;

IBC_API wire_handle wire_new(int32_t capacity);
IBC_API void wire_delete(wire_handle handle);

IBC_API void wire_segment_origin(wire_handle handle, float vec[3], float color[3]);
IBC_API void wire_segment(wire_handle handle, float vec[3], float origin[3], float color[3]);
IBC_API void wire_grid(wire_handle handle, uint32_t segments, int mode);
IBC_API void wire_axis(wire_handle handle, float pos[3]);

IBC_API uint64_t wire_get_position(wire_handle handle);
IBC_API void wire_clear(wire_handle handle, uint64_t position);
IBC_API void wire_clear_all(wire_handle handle);

IBC_API void wire_draw(wire_handle handle, float projection[16], float view[16]);
IBC_API void wire_apply(wire_handle handle);

#endif //IBCWEB_WIRE_H
