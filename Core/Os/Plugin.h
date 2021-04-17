/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef PLUGIN_H
#define PLUGIN_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef struct plg* plg_handle;

typedef struct plg_desc{
    char* name;
    int32_t min_version;
} plg_desc;

typedef struct plg_info{
    char* name;
    int32_t version;
    int32_t req_plugins_count;
    plg_desc* req_plugins;
} plg_info;

API plg_handle plg_load(plg_desc const*);
API void plg_update(plg_handle);
API void plg_unload();

#endif //PLUGIN_H
