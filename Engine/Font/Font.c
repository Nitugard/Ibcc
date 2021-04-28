/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Font.h"
#include "Arial.h"
#include <string.h>


//todo: preload texture with stbi
void * font_texture_get() {
    return font_Arial_Unicode_MS_file;
}

void font_texture_info(int32_t* width, int32_t* height, int32_t* channels)
{
    *width = font_Arial_Unicode_MS_file_width;
    *height = font_Arial_Unicode_MS_file_height;
    *channels = font_Arial_Unicode_MS_file_channels;
}

Character* font_get_char(char x) {

    //todo: 1 - 1 index
    for (int32_t i = 0; i < font_Arial_Unicode_MS.characterCount; ++i) {
        if (font_Arial_Unicode_MS.characters[i].codePoint == (int32_t) x)
        {
            return font_Arial_Unicode_MS.characters + i;
        }
    }
    return 0;
}

void font_quad_vertex_set(font_vertex* vertex, float x, float y, float ux, float uy) {
    vertex->pos[0] = x;
    vertex->pos[1] = y;
    vertex->uv[0] = ux;
    vertex->uv[1] = uy;
}

void font_quad_set(struct font_quad* p0, float x, float y, Character c,
                   float width, float height) {

    // p2 --- p3
    // | \     |
    // |   \   |
    // |     \ |
    // p0 --- p1

    float x0 = x - c.originX;
    float y0 = y + c.originY;
    float s0 = c.x / width;
    float t0 = c.y / height;

    float x1 = x - c.originX + c.width;
    float y1 = y + c.originY;
    float s1 = (c.x + c.width) / width;
    float t1 = c.y / height;

    float x2 = x - c.originX;
    float y2 = y + c.originY - c.height;
    float s2 = c.x / width;
    float t2 = (c.y + c.height) / height;

    float x3 = x - c.originX + c.width;
    float y3 = y + c.originY - c.height;
    float s3 = (c.x + c.width) / width;
    float t3 = (c.y + c.height) / height;

    font_quad_vertex_set(p0->vertex + 2, x0, y0, s0, t0);
    font_quad_vertex_set(p0->vertex + 1, x1, y1, s1, t1);
    font_quad_vertex_set(p0->vertex + 0, x3, y3, s3, t3);

    font_quad_vertex_set(p0->vertex + 5, x0, y0, s0, t0);
    font_quad_vertex_set(p0->vertex + 4, x3, y3, s3, t3);
    font_quad_vertex_set(p0->vertex + 3, x2, y2, s2, t2);
}

font_result font_create(const char *label, struct font_quad *vertices) {
    int32_t len = strlen(label);
    int32_t cursor_x = 0;

    for (int32_t i = 0; i < len; ++i) {
        Character *char_data = font_get_char(label[i]);
        font_quad_set(vertices + i, cursor_x, 0, *char_data, font_Arial_Unicode_MS.width, font_Arial_Unicode_MS.height);
        cursor_x += char_data->advance;
    }
    struct font_result res = {.width = cursor_x, .height = font_Arial_Unicode_MS.size};
    return res;
}

