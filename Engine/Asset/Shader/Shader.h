/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SHADER_H
#define IBC_SHADER_H

#include <Common.h>

typedef struct shader_t* shader_hndl;

typedef struct shader_attribute{
    const i8* name;
    const i8* type;
    const i8* location;
    const i8* semantic;
    i32 elements;
} shader_attribute;

typedef struct shader_uniform{

} shader_uniform;

typedef struct shader_t{

    const i8* name;
    i32 uniforms_count;
    shader_uniform const* uniforms;
    i32 attributes_count;
    shader_attribute const* attributes;

} shader_t;

API i32 shader_attribute_find_by_name(shader_hndl hndl, i8 const* name);
API i32 shader_uniform_find_by_name(shader_hndl hndl, i8 const* name);

#endif //IBC_SHADER_H
