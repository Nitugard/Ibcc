/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <Texture/Texture.h>

typedef enum buffer_flag {
    BUFFER_FLAG_VERTEX = 1,
    BUFFER_FLAG_COLOR = 2,
    BUFFER_FLAG_UV = 4,
    BUFFER_FLAG_NORMAL = 8,
} buffer_flag;

typedef struct model_t {
    unsigned int buffer_length;
    void *buffer;
    buffer_flag flag;
    tex_handle texture;
} model_t;

#endif //IBC_MODEL_H
