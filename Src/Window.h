/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_WINDOW_H
#define IBCWEB_WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#ifndef IBC_API
#define IBC_API extern
#endif

#define MAXIMUM_LOADED_SHADERS 32

typedef struct window_config{
    int32_t width, height;
    bool fullscreen;
    bool vsync;
    bool resizable;
    char* title;
}window_config;

IBC_API void window_init(window_config const* config);
IBC_API void window_run();
IBC_API void window_finalize();

#endif //IBCWEB_WINDOW_H
