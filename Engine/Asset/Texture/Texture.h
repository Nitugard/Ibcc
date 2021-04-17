/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_TEXTURE_H
#define IBC_TEXTURE_H

typedef struct tex_data{
    int width;
    int height;
    int num_channels;
    void* data;
} tex_data;

typedef struct tex_data* tex_handle;

#endif //IBC_TEXTURE_H
