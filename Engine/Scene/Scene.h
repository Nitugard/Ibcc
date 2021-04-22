/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SCENE_H
#define IBC_SCENE_H

#ifndef API
#define API
#endif

//Todo: this is a high level and deserves special folder with game
//Todo: add more scene description such as shader

#include "Asset/Model/Model.h"
#include "Graphics/Graphics.h"

typedef struct scene_data* scene_handle;

API scene_handle scene_new(gfx_buffer_handle camera_handle, xmdl_data xmdl);
API void scene_draw(scene_handle handle);
API void scene_delete(scene_handle handle);

#endif //IBC_SCENE_H
