/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_SKYBOX_H
#define IBCWEB_SKYBOX_H

#ifndef IBC_API
#define IBC_API extern
#endif

typedef struct skybox_data* skybox_handle;

IBC_API skybox_handle skybox_load(const char* path);
IBC_API void skybox_render(skybox_handle handle, float projection[16], float view[16]);
IBC_API void skybox_bind(skybox_handle handle);
IBC_API void skybox_destroy(skybox_handle handle);

#endif //IBCWEB_SKYBOX_H
