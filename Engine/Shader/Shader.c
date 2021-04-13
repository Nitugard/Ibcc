/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Shader.h"

#include <Plugin/Plugin.h>
#include <Asset/Asset.h>

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Json";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    asset_register_desc json_desc = {
            .extension = "json",
            .asset_on_load = asset_on_load_shader,
            .asset_on_unload = asset_on_unload_shader
    };

    return true;
}

void plg_on_stop(plg_info* info)
{
}
