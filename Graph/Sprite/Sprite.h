/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */



#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <stdbool.h>

#ifndef API
#define API
#endif

typedef struct sprite_shader{
    bool enabled;
    void* gfx_shader_handle;
} sprite_shader;

typedef struct sprite_desc{
    int32_t width;
    int32_t height;
    int32_t offset_x;
    int32_t offset_y;
    float scale;
    void* gfx_texture_handle;
    sprite_shader custom_shader;
} sprite_desc;

typedef struct sprite_data* sprite_handle;

API void sprite_init(int32_t capacity);
API void sprite_terminate();

API sprite_handle sprite_new(sprite_desc desc);
API void sprite_update(sprite_handle handle, sprite_desc desc);
API void sprite_delete(sprite_handle handle);

API void sprite_draw();


#endif //SPRITE_H
