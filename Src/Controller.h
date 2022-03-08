/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_CONTROLLER_H
#define IBCWEB_CONTROLLER_H

#include <stdbool.h>

#ifndef IBC_API
#define IBC_API extern
#endif

typedef struct controller_data* controller_handle;

IBC_API controller_handle controller_create(float pos[16], float move_speed, float rotate_speed);
IBC_API bool controller_update(controller_handle handle);
IBC_API void controller_set_transform(controller_handle handle, float* world_tr);
IBC_API void controller_get_transform(controller_handle handle, float* world_tr);
IBC_API void controller_destroy(controller_handle handle);
IBC_API void controller_get_origin(controller_handle handle, float* origin);

#endif //IBCWEB_CONTROLLER_H
