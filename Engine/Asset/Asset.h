/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ASSET_H
#define FIXEDPHYSICS_ASSET_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef struct asset* asset_hndl;

typedef struct asset_register_desc{
    char const extension[32];
    asset_hndl(*asset_on_load)(char* path);
    void(*asset_on_unload)(asset_hndl asset);
} asset_register_desc;

API bool asset_register(struct asset_register_desc const* desc);

API bool asset_exists(char const* name);
API asset_hndl asset_load(char const* name);
API void asset_unload(asset_hndl hndl);

#endif //FIXEDPHYSICS_ASSET_H
