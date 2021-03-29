/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <Plugin/Plugin.h>
#include <Log/Log.h>
#include <Os/Allocator.h>

int main() {

    os_allocator_init();

    plg_desc desc = {
            .name = "Game",
            .min_version = 1
    };
    plg_handle plg = plg_load(&desc);
    os_allocator_finalize();

    return 0;
}