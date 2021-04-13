/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_CUBE_H
#define IBC_CUBE_H

#define CUBE_TRIANGLES sizeof(cube_indices) / sizeof(float)

u32 cube_indices[] = {
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3

};

f32 cube_vertices[] = {
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        // back
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0

};


f32 cube_uv[] = {

};

float cube_colors[] = {
        1.0f, 0, 0, 1,
        0, 1.0f, 0, 1,
        0, 0, 1.0f, 1,
        1.0f, 1.0f, 0, 1,
        0, 1.0f, 1.0f, 1,
        1.0f, 0, 1.0f, 1,
        1.0f, 1.0f, 1.0f, 1,
        0, 0, 0, 1,
};

#endif //IBC_CUBE_H
