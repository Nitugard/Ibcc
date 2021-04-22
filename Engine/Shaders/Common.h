/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_COMMON_H
#define IBC_COMMON_H

#define position_attr 0
#define color_attr 1
#define uv_attr 2
#define normal_attr 3

#define SHADER_VERSION #version 330\n

#define SHADER_MATRICES \
    uniform mat4 model;                    \
    layout(std140, row_major) uniform matrices{ \
        mat4 view; \
        mat4 projection; \
        vec3 light_pos; \
        vec3 cam_pos; \
    };

typedef struct SH_MVP_T{
    float view[16];
    float projection[16];

    float light_pos[3];
    float __pad1;
    float cam_pos[3];
    float __pad2;
} SH_MVP_T;

#define __STRING(x) #x
#define STRING(x) __STRING(x)

#endif //IBC_COMMON_H
