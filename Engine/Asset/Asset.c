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
#define MAXIMUM_LOADED_ASSETS 1024

typedef struct asset_data{
    asset_hndl hndl;
    const i8* name;
} asset_data;

bool asset_descriptions_valid[MAXIMUM_ASSETS];
asset_register_desc asset_descriptions[MAXIMUM_ASSETS];
asset_data loaded_assets[MAXIMUM_LOADED_ASSETS];

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Asset";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    OS_MEMSET(asset_descriptions_valid, 0, sizeof(bool) * MAXIMUM_ASSETS);
    OS_MEMSET(loaded_assets, 0, sizeof(asset_data) * MAXIMUM_LOADED_ASSETS);

    return true;
}

void plg_on_stop(plg_info* info)
{
}

i32 asset_config_find_by_id(i8* const fext)
{
    i32 i;
    for(i=0; i<MAXIMUM_ASSETS; ++i)
    {
        if(!asset_descriptions_valid[i]) continue;
        if(strcmp(asset_descriptions[i].extension, fext) == 0)
            return i;
    }

    return -1;
}

i32 asset_data_find_by_path(const i8* path)
{
    for (i32 i = 0; i < MAXIMUM_LOADED_ASSETS; ++i) {
        if (loaded_assets[i].name == path) {
            return i;
        }
    }
    return -1;
}

i32 asset_data_find_by_hndl(asset_hndl hndl)
{
    for (i32 i = 0; i < MAXIMUM_LOADED_ASSETS; ++i) {
        if (loaded_assets[i].hndl == hndl) {
            return i;
        }
    }
    return -1;
}

bool asset_loaded_data_full() {
    for (i32 i = 0; i < MAXIMUM_LOADED_ASSETS; ++i) {
        if (loaded_assets[i].hndl == 0) {
            return false;
        }
    }

    return true;
}

bool asset_register(struct asset_register_desc const* desc) {
    for (i32 i = 0; i < MAXIMUM_ASSETS; ++i) {
        if(!asset_descriptions_valid[i]) continue;
        if (strcmp(asset_descriptions[i].extension, desc->extension) == 0) {
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

    i32 i = asset_config_find_by_id(fext);
    if(i == -1) {
        LOG_ERROR("Configuration for the asset type could not be found %s\n", path);
        return false;
    }

    file_hndl fhndl = file_open(path, "r");
    *((asset_hndl*)data) = asset_descriptions[i].asset_on_load(fhndl);
    file_close(fhndl);

    return true;
}

asset_hndl asset_load(const i8 *name) {

    i32 pi = asset_data_find_by_path(name);
    if(pi != -1) {
        LOG_INFO("Asset preloaded %s\n", name);
        return loaded_assets[pi].hndl;
    }

    if(asset_loaded_data_full())
    {
        LOG_ERROR("Asset could not be loaded %s, capacity is full\n", name);
        return 0;
    }

    asset_hndl hndl = NULL;
    if(!asset_load_ex(name, &hndl))
    {
        LOG_ERROR("Asset could not be loaded %s\n", name);
    }
    else {
        LOG_INFO("Asset loaded %s\n", name);
        i32 i;
        for (i = 0; i < MAXIMUM_LOADED_ASSETS; ++i) {
            if (loaded_assets[i].hndl == 0) {
                loaded_assets[i].hndl = hndl;
                loaded_assets[i].name = name;
                break;
            }
        }
    }

    return hndl;
}

void asset_unload(asset_hndl hndl) {
    i32 i = asset_data_find_by_hndl(hndl);
    if (i == -1) {
        LOG_FATAL("Asset could not be unloaded!\n");
        return;
    }

    i8 fname[1024];
    i8 fext[128];
    file_path_filename(loaded_assets[i].name, fname, true);
    file_path_ext(fname, fext);

    i32 extId = asset_config_find_by_id(fext);
    asset_descriptions[extId].asset_on_unload(hndl);

    loaded_assets[i].name = "\0";
    loaded_assets[i].hndl = 0;
}

