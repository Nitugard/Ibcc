/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <Os/Plugin.h>
#include <Os/Allocator.h>
#include <Os/Log.h>

#include <malloc.h>

int main() {
    plg_desc desc = {
            .name = "Game",
            .min_version = 1
    };
    os_allocator_init();
    plg_load(&desc);
    plg_unload();
    uint32_t tracked_allocations = os_get_tracked_allocations_length();
    if(tracked_allocations > 0) {
        const os_proxy_header **allocations = malloc(sizeof(struct os_proxy_header *) * tracked_allocations);
        os_get_tracked_allocations(allocations);
        for (unsigned int i = 0; i < tracked_allocations; ++i) {
            const struct os_proxy_header *data = allocations[i];
            LOG_ERROR("Leak detected: %i %s:%i", data->size, data->file, data->line);
        }
    }

    os_allocator_finalize();
    return 0;
}