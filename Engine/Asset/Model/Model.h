/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <stdint.h>
#include <stdbool.h>



typedef struct vertex_t{
    float pos[3];
    float color[3];
    float uv[3];
    float normal[3];
} vertex_t;

typedef struct mdl_material {

} mdl_material;

typedef struct mdl_data
{
    float* buffer;
    uint32_t buffer_size;
    uint32_t vertices;
} mdl_data;

typedef mdl_data* mdl_data_handle;

#endif //IBC_MODEL_H
