/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_GRAPHICS_H
#define FIXEDPHYSICS_GRAPHICS_H

#include "../Types.h"

#ifndef GFX_ASSERT
#include <assert.h>
#define GFX_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

#ifndef API
#define API extern
#endif

typedef struct gfx_shader_attr{

} gfx_shader_attr;

typedef struct gfx_shader_stage{
    i8 const* src;
    i8 const* entry;
    i32 length;
} gfx_shader_stage;

typedef struct gfx_shader_desc{
    gfx_shader_stage fs, vs;
    i8 const* name;
    gfx_shader_attr attrs[128];

} gfx_shader_desc;

typedef struct gfx_pipeline_desc{

} gfx_pipeline_desc;

typedef struct gfx_shader* gfx_shader_handle;
typedef struct gfx_pipeline gfx_pipeline_handle;

API gfx_shader_handle gfx_shader_create(gfx_shader_desc const* desc);
API void gfx_shader_destroy(gfx_shader_handle);

API void gfx_create_buffer();
API void gfx_destroy_buffer();

API gfx_pipeline_handle gfx_pipeline_create(gfx_pipeline_desc const* desc);
API void gfx_pipeline_destroy();

#endif //FIXEDPHYSICS_GRAPHICS_H
