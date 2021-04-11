/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Asset.h"
#include <Plugin/Plugin.h>

extern void asset_init();
extern asset_hndl asset_on_load_json(void*);
extern asset_hndl asset_on_load_shader(void*);

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Asset";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    asset_init();
    asset_register_desc json_desc = {.extension = "json", .asset_on_load = asset_on_load_json};
    asset_register(&json_desc);
    asset_register_desc shader_desc = {.extension = "glsl", .asset_on_load = asset_on_load_shader};
    asset_register(&shader_desc);

    return true;
}

void plg_on_stop(plg_info* info)
{
}
