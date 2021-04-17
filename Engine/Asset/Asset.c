/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Asset.h"

#include <Os/Plugin.h>
#include <Os/File.h>
#include <Os/Log.h>
#include <Containers/String.h>
#include <Containers/Array.h>

typedef struct asset_data{
    asset_hndl hndl;
    string_handle path;
} asset_data;

arr_handle registered_types; //asset_register_desc[]
arr_handle loaded_assets; //asset_data[]

extern void init_json_asset();
extern void init_model_asset();
extern void init_texture_asset();

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Asset";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {

    registered_types = arr_new(sizeof(asset_register_desc), 32);
    loaded_assets = arr_new(sizeof(struct asset_data), 32);

    init_json_asset();
    init_model_asset();
    init_texture_asset();

    return true;
}

void plg_on_stop(plg_info* info) {
    while(arr_size(loaded_assets) > 0) {
        asset_unload(((struct asset_data *) arr_get(loaded_assets, 0))->hndl);
    }
    arr_delete(registered_types);
    arr_delete(loaded_assets);
}

int32_t find_registered_type_description(char const* fext)
{
    int32_t i;
    for(i=0; i < arr_size(registered_types); ++i) {
        struct asset_register_desc const *desc = (struct asset_register_desc const*) arr_get(registered_types, i);
        if (str_cmp(desc->extension, fext) == 0)
            return i;
    }
    return -1;
}

int32_t asset_data_find_by_path(const char* path)
{
    int32_t i;
    for(i=0; i < arr_size(loaded_assets); ++i) {
        struct asset_data const *data= (struct asset_data const*) arr_get(loaded_assets, i);
        if (str_cmp(str_internal(data->path), path) == 0)
            return i;
    }
    return -1;

}

int32_t asset_data_find_by_hndl(asset_hndl hndl)
{
    int32_t i;
    for(i=0; i < arr_size(loaded_assets); ++i) {
        struct asset_data const *data= (struct asset_data const*) arr_get(loaded_assets, i);
        if (data->hndl == hndl)
            return i;
    }
    return -1;
}

bool asset_register(struct asset_register_desc const* desc) {
    int32_t index = find_registered_type_description(desc->extension);
    if (index != -1) return false;
    asset_register_desc desc_nonconst = *desc;
    arr_add(registered_types, &desc_nonconst);
    return true;
}

bool asset_load_execute(const char* path, void* data) {
    string_handle path_handle = str_new(path);
    string_handle extension_handle = str_path_ext(path_handle);

    int32_t i = find_registered_type_description(str_internal(extension_handle));

    str_delete(path_handle);
    str_delete(extension_handle);

    if (i == -1) {
        LOG_ERROR("Configuration for the asset type could not be found %s\n", path);
        return false;
    }

    *((asset_hndl *) data) = ((struct asset_register_desc const *) arr_get(registered_types, i))->asset_on_load(path);
    return true;
}

asset_hndl asset_load(const char *name) {

    int32_t pi = asset_data_find_by_path(name);
    if (pi != -1) {
        LOG_INFO("Asset preloaded %s\n", name);
        return ((struct asset_data *const) arr_get(loaded_assets, pi))->hndl;
    }

    asset_hndl hndl = 0;
    if (!asset_load_execute(name, &hndl)) {
        LOG_ERROR("Asset could not be loaded %s\n", name);
    } else {
        LOG_INFO("Asset loaded %s\n", name);
        asset_data data = {.hndl = hndl, .path = str_new(name)};
        arr_add(loaded_assets, &data);
    }

    return hndl;
}

void asset_unload(asset_hndl hndl) {
    int32_t i = asset_data_find_by_hndl(hndl);
    if (i == -1) {
        LOG_FATAL("Asset could not be unloaded!\n");
        return;
    }
    asset_data const *data = ((struct asset_data *const) arr_get(loaded_assets, i));

    string_handle path_handle = data->path;
    string_handle extension_handle = str_path_ext(path_handle);

    int32_t extId = find_registered_type_description(str_internal(extension_handle));
    ((struct asset_register_desc const *) arr_get(registered_types, extId))->asset_on_unload(hndl);

    str_delete(path_handle);
    str_delete(extension_handle);

    arr_remove_swap(loaded_assets, i);
}

