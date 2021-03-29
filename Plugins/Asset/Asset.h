/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ASSET_H
#define FIXEDPHYSICS_ASSET_H

#include <Common.h>

typedef int asset_type;
typedef void asset;

typedef struct {
    char const* path;
    asset* asset;
    unsigned int timestamp;
} asset_hndl;

typedef struct {

} asset_init_desc;

API int asset_hndl_valid(asset const*);
API const char* asset_hndl_path(asset const*);
API asset* asset_hndl_ptr(asset const*);
API int asset_hndl_equal(asset const*);

API void asset_init(asset_init_desc const*);
API void asset_finish(void);
API void asset_cache_flush(void);
//API void asset_register(asset_type, char const*, asset*(*)(file_hndl const*), void(*)(asset const*));


API asset* asset_get(char const*);
API asset* asset_get_load(char const*);
API asset* asset_get_load_as_type(asset_type, char const*);

API char const* asset_get_path(asset const*);
API char const* asset_get_extension(asset const*);
API char const* asset_get_typename(asset const*);

#endif //FIXEDPHYSICS_ASSET_H
