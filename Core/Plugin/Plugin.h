/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_PLUGIN_H
#define FIXEDPHYSICS_PLUGIN_H

#include <Common.h>

typedef struct plg* plg_handle;

typedef struct plg_desc{
    i8* name;
    i32 min_version;
} plg_desc;

typedef struct plg_info{
    i8* name;
    i32 version;
    i32 req_plugins_count;
    plg_desc* req_plugins;
} plg_info;

API plg_handle plg_load(plg_desc const*);
API void plg_update(plg_handle);
API void plg_unload();

#endif //FIXEDPHYSICS_PLUGIN_H