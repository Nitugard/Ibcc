/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <Os/Allocator.h>
#include <Os/Log.h>

#include "Game/Game.h"

#include <malloc.h>

int main() {
    os_allocator_init();

    game_init();

    uint32_t tracked_allocations = os_get_tracked_allocations_length();
    if(tracked_allocations > 0) {
        const os_proxy_header **allocations = malloc(sizeof(struct os_proxy_header *) * tracked_allocations);
        os_get_tracked_allocations(allocations);
        for (unsigned int i = 0; i < tracked_allocations; ++i) {
            const struct os_proxy_header *data = allocations[i];
            LOG_ERROR("Leak detected: %i %s:%i\n", data->size, data->file, data->line);
        }
    }

    os_allocator_finalize();
    return 0;
}