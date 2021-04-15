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

#ifdef PLUGIN_IMPLEMENTATION
#include <windows.h>
#include "Log.h"

#ifndef PLUGIN_ASSERT
#include <assert.h>
#define PLUGIN_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef void (*proc_on_start)(plg_info*);
typedef void (*proc_on_stop)();
typedef bool (*proc_on_load)();
typedef void (*proc_on_unload)();
typedef void (*proc_on_update)();

typedef struct plg {

    HMODULE handle;

    //only called once
    proc_on_start proc_start;
    proc_on_stop proc_stop;

    //called when loaded/unloaded(reload)
    proc_on_load proc_load;
    proc_on_unload proc_unload;
    proc_on_update proc_update;

    plg_info info;
    bool initialized;
} plg;

#define MAXIMUM_PLUGINS_COUNT 256
plg plugins[MAXIMUM_PLUGINS_COUNT];
plg_handle plugins_graph[MAXIMUM_PLUGINS_COUNT];

int32_t loaded_plugins_count = 0;
int32_t graph_count = 0;

int32_t plg_get_index_by_name(const char* name)
{
    for(int32_t i=0; i < loaded_plugins_count; ++i)
    {
        if(strcmp(plugins[i].info.name, name) == 0)
            return i;
    }

    return -1;
}

void plg_dependecy_resolve(int32_t node) {
    plg_info info = plugins[node].info;
    for (int32_t i = 0; i < info.req_plugins_count; ++i) {
        plg_dependecy_resolve(plg_get_index_by_name(info.req_plugins[i].name));
    }

    for (int32_t i = 0; i < graph_count; ++i) {
        if (strcmp(plugins_graph[i]->info.name, plugins[node].info.name) == 0)
            return;
    }
    plugins_graph[graph_count++] = plugins + node;
}

void plg_update_dependency_graph()
{
    graph_count = 0;
    plg_dependecy_resolve(loaded_plugins_count-1);
}

plg_handle plg_load_recursive(const plg_desc * desc) {

    //check if plugins has already been loaded
    int32_t loaded_plugin_id = plg_get_index_by_name(desc->name);
    if(loaded_plugin_id != -1)
    {
        //todo: check version and print error if mismatched
        return plugins + loaded_plugin_id;
    }

    HMODULE lib = LoadLibrary(desc->name);
    if (lib != NULL) {
        plg_info info;

        FARPROC proc_start =  GetProcAddress(lib, "plg_on_start");
        FARPROC proc_stop =  GetProcAddress(lib, "plg_on_stop");
        FARPROC proc_load =  GetProcAddress(lib, "plg_on_load");
        FARPROC proc_unload =  GetProcAddress(lib, "plg_on_unload");
        FARPROC proc_update =  GetProcAddress(lib, "plg_on_update");

        if(proc_start == 0) {
            LOG_ERROR("Plugin load failed for %s, procedure void(*plg_on_start)(plg_info*) not found.\n", desc->name);
            return 0;
        }


        (void(*)(plg_info*))(void*)(proc_start)(&info);
        if(strcmp(info.name, desc->name) != 0)
        {
            LOG_ERROR("Plugin load failed for %s, plg_name mismatch (%s).\n", desc->name, info.name);
            return 0;
        }

        //check version
        if(info.version < desc->min_version) {
            LOG_ERROR("Could not load plugin %s, version mismatch, required: %i, current: %i\n", desc->name,
                      desc->min_version, info.version);
            return 0;
        }

        for(int32_t i=0; i < info.req_plugins_count; ++i) {
            if (!plg_load_recursive(&info.req_plugins[i])) {
                LOG_ERROR("Plugin load failed for %s, dependency error\n", info.name);
                return 0;
            }
        }

        plg plugin = {
                .info = info,
                .proc_start = (proc_on_start)(void*)proc_start,
                .proc_stop = (proc_on_stop)(void*)proc_stop,
                .proc_load = (proc_on_load)(void*)proc_load,
                .proc_unload = (proc_on_unload)(void*)proc_unload,
                .proc_update = (proc_on_update)(void*)proc_update,
                .initialized = false,
                .handle = lib,
        };
        plugins[loaded_plugins_count++] = plugin;
        return plugins+(loaded_plugins_count-1);
    }

    LOG_ERROR("Plugin load failed for %s, could not be found", desc->name);
    return 0;
}

plg_handle plg_load(const plg_desc * desc) {
    plg_handle handle = plg_load_recursive(desc);
    if(handle != 0) {
        plg_update_dependency_graph();

        for(int32_t i=0; i < loaded_plugins_count; ++i)
        {
            if(!plugins_graph[i]->initialized)
            {
                LOG_INFO("Plugin %s loaded, version: %i loaded, dependency: %i\n", plugins_graph[i]->info.name,
                         plugins_graph[i]->info.version, plugins_graph[i]->info.req_plugins_count);

                if(!plugins_graph[i]->proc_load(&plugins_graph[i]->info))
                {
                    //plugin failed to be loaded, everything that depends on this plugin
                    //will be unloaded
                    //TODO
                }
                plugins_graph[i]->initialized = true;
            }
        }
    }

    return handle;
}

void plg_stop(plg_handle handle) {
    //check if anything depends on this plugin, fail if does
    handle->proc_stop(&handle->info);
    handle->initialized = false;
    handle->handle = 0;
    FreeLibrary(handle->handle);
    //TODO: REMOVE FROM PLUGINS_GRAPH
}

void plg_update(plg_handle handle) {
    for (int32_t i = 0; i < loaded_plugins_count; ++i) {
        if(plugins_graph[i]->proc_update != 0) plugins_graph[i]->proc_update();
        if (handle == plugins_graph[i])
            return;
    }
}

void plg_unload() {

    for (int32_t i = 0; i < loaded_plugins_count; ++i) {
        plg_stop(plugins_graph[i]);
    }

    loaded_plugins_count = 0;
}

#undef PLUGIN_IMPLEMENTATION
#endif