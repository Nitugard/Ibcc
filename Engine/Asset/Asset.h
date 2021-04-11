/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ASSET_H
#define FIXEDPHYSICS_ASSET_H

#include <Common.h>

typedef int asset_type;
typedef struct asset* asset_hndl;

typedef struct asset_register_desc{
    i8 const* extension;
    asset_hndl(*asset_on_load)(void* file);
} asset_register_desc;

API bool asset_register(struct asset_register_desc const* desc);

API bool asset_exists(i8 const* name);
API asset_hndl asset_load(i8 const* name);
API void asset_unload(asset_hndl hndl);

#endif //FIXEDPHYSICS_ASSET_H
