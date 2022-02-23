/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <stdbool.h>

#ifndef API
#define API
#endif
typedef struct text_desc{
    int32_t offset_x;
    int32_t offset_y;
    float scale;
    const char* label;
} text_desc;

typedef struct text_data* text_handle;

API void text_init();
API void text_terminate();

API text_handle text_new(int32_t capacity);
API void text_update(text_handle handle, text_desc* desc);
API void text_delete(text_handle handle);

API void text_draw(text_handle handle);


#endif //TEXT_H
