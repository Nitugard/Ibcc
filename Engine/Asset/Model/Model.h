/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct mdl_data
{
    float* buffer; //vertex, uv, color, normal
    uint32_t stride;
    uint32_t buffer_size;

} mdl_data;

typedef mdl_data* mdl_data_handle;

#endif //IBC_MODEL_H
