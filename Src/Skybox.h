/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_SKYBOX_H
#define IBCWEB_SKYBOX_H

#include <stdint.h>

#ifndef IBC_API
#define IBC_API extern
#endif

typedef struct skybox_data* skybox_handle;

IBC_API skybox_handle skybox_load(const char* path);
IBC_API void skybox_render(skybox_handle handle, float projection[16], float view[16]);
IBC_API void skybox_bind(skybox_handle handle);
IBC_API float skybox_get_exposure(skybox_handle handle);
IBC_API void skybox_set_exposure(skybox_handle handle, float exposure);
IBC_API void skybox_destroy(skybox_handle handle);

/* Returns the raw OpenGL texture ID of the skybox's environment cubemap.
 * Pass to prefilter_env_generate() to build the specular IBL map. */
IBC_API uint32_t skybox_get_cubemap_id(skybox_handle handle);

#endif //IBCWEB_SKYBOX_H
