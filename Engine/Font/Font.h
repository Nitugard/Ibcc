/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stdbool.h>

#ifndef API
#define API
#endif

typedef struct font_result{
    int32_t width;
    int32_t height;
} font_result;

typedef struct font_vertex{
    float pos[2];
    float uv[2];
} font_vertex;

typedef struct font_quad{
    font_vertex vertex[6];
} font_quad;

typedef struct font_data* font_handle;

API void font_init();
API void font_terminate();

API void font_texture_info(int32_t* width, int32_t* height, int32_t* channels);
API void* font_texture_get();
API font_result font_create(char const* label, font_quad* quads);

#endif //FONT_H
