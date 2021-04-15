/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <Os/Plugin.h>
#include <Os/Allocator.h>
int main() {
    plg_desc desc = {
            .name = "Game",
            .min_version = 1
    };
    os_allocator_init();
    plg_load(&desc);
    plg_unload();
    os_allocator_finalize();
    return 0;
}