/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Texture.h"

#include <Os/Allocator.h>
#include <Os/File.h>
#include <Asset/Asset.h>
#include <Os/Plugin.h>
#include <Os/Log.h>


#define STB_IMAGE_IMPLEMENTATION
//#define STBI_MALLOC(size) OS_MALLOC(size)
//#define STBI_REALLOC(x, size) OS_REALLOC(x, size)
//#define STBI_FREE(x) OS_FREE(x)
#include <ThirdParty/stb_image/stb_image.h>

asset_hndl asset_on_load_tex(void* hptr) {
    file_hndl hndl = (file_hndl) hptr;
    tex_data *data = OS_MALLOC(sizeof(tex_data));
    data->data = stbi_load(file_path(hndl), &(data->width), &(data->height), &(data->num_channels), 0);
    if(data->data == 0) {
        LOG_ERROR("Texture load failed\n");
    }

    return (asset_hndl) data;
}

void asset_on_unload_tex(asset_hndl hndl)
{
    stbi_image_free(((struct tex_data*)hndl)->data);
    OS_FREE(hndl);
}

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Texture";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    asset_register_desc jpg_desc = {
            .extension = "png",
            .asset_on_load = asset_on_load_tex,
            .asset_on_unload = asset_on_unload_tex
    };
    asset_register(&jpg_desc);
    return true;
}

void plg_on_stop(plg_info* info)
{
}
