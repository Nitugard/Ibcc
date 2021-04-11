/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Asset.h"
#include <Plugin/Plugin.h>
#include <Os/Allocator.h>
#include <Os/File.h>
#include <string.h>
#include <Log/Log.h>

#define MAXIMUM_ASSETS 128
#define ASSET_FOLDER "./Assets/"

bool asset_descriptions_valid[MAXIMUM_ASSETS];
asset_register_desc asset_descriptions[MAXIMUM_ASSETS];

void asset_init()
{
    OS_MEMSET(asset_descriptions_valid, 0, sizeof(bool) * MAXIMUM_ASSETS);
}

bool asset_find_execute(const i8* name, void* data, bool(*execute)(i8 const* path, void* data))
{
    char buffer[1024 * 10];
    OS_MEMSET(buffer, 0, 1024 * 10);
    strcat(buffer, ASSET_FOLDER);
    strcat(buffer, name);
    if(file_exists(buffer))
    {
        if(execute != 0) return execute(buffer, data);
    }

    return false;
}

bool asset_register(struct asset_register_desc const* desc) {
    for (i32 i = 0; i < MAXIMUM_ASSETS; ++i) {
        if(!asset_descriptions_valid[i]) continue;
        if (strcmp(asset_descriptions[i].extension, desc->extension) != 0) {
            return false;
        }
    }
    for (i32 i = 0; i < MAXIMUM_ASSETS; ++i) {
        if(!asset_descriptions_valid[i])
        {
            asset_descriptions_valid[i] = true;
            asset_descriptions[i] = *desc;
            return true;
        }
    }
    return false;
}

bool asset_load_ex(const i8* path, void* data)
{
    i8 fname[1024];
    i8 fext[128];
    file_path_filename(path, fname, true);
    file_path_ext(fname, fext);

    for(i32 i=0; i<MAXIMUM_ASSETS; ++i)
    {
        if(!asset_descriptions_valid[i]) continue;
        if(strcmp(asset_descriptions[i].extension, fext) == 0)
        {
            file_hndl fhndl = file_open(path, "r");
            *((asset_hndl*)data) = asset_descriptions[i].asset_on_load(fhndl);
            file_close(fhndl);
            return true;
        }
    }

    return false;
}

asset_hndl asset_load(const i8 *name) {
    asset_hndl hndl = NULL;
    if(!asset_find_execute(name, &hndl, asset_load_ex))
    {
        LOG_ERROR("Asset could not be loaded %s\n", name);
    }
    else{
        LOG_INFO("Asset loaded %s\n", name);
    }

    return hndl;
}

bool asset_exists(const i8 *name) {

    return asset_find_execute(name, NULL, NULL);
}


