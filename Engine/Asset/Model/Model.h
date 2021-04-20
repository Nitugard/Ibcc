/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <stdint.h>
#include <stdbool.h>


#define MDL_ELEMENT_SIZE 4

#define MDL_STRIDE_POS 3
#define MDL_STRIDE_COLOR 3
#define MDL_STRIDE_UV 2
#define MDL_STRIDE_NORMAL 3

#define MDL_OFFSET_POS 0
#define MDL_OFFSET_COLOR 3
#define MDL_OFFSET_UV 6
#define MDL_OFFSET_NORMAL 8

#define MDL_EL_SIZE(count) (MDL_ELEMENT_SIZE * (count))
#define MDL_STRIDE (MDL_STRIDE_POS + MDL_STRIDE_COLOR + MDL_STRIDE_UV + MDL_STRIDE_NORMAL)
#define MDL_STRIDE_SIZE MDL_EL_SIZE(MDL_STRIDE)

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
