/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_GRAPHICS_H
#define FIXEDPHYSICS_GRAPHICS_H

#include <Common.h>

#define MAXIMUM_PIPELINE_ATTRIBUTES 32
#define MAXIMUM_PIPELINE_UNIFORM_BLOCKS 8

typedef enum gfx_buffer_type {
    VERTEX,
    INDEX,
    UNIFORM,
} gfx_buffer_type;

typedef enum gfx_buffer_update_mode {
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,
    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,
    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY,
} gfx_buffer_update_mode;

typedef enum gfx_pass_action_type
{
    GFX_ACTION_CLEAR,
} gfx_pass_action_type;

typedef struct gfx_shader *gfx_shader_handle;
typedef struct gfx_pipeline *gfx_pipeline_handle;
typedef struct gfx_buffer *gfx_buffer_handle;

typedef struct gfx_color{
    float x,y,z,w;
} gfx_color;

typedef struct gfx_shader_attr {
    i32 size;
    i32 num_elements;
} gfx_shader_attr;

typedef struct gfx_pipeline_attr {
    i8 enabled;
    i32 offset;
    i32 stride;
    gfx_buffer_handle buffer;
} gfx_pipeline_attr;

typedef struct gfx_shader_stage {
    i8 const *src;
} gfx_shader_stage;

typedef struct gfx_shader_desc {
    gfx_shader_stage fs, vs;
    i8 const *name;
    gfx_shader_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];

} gfx_shader_desc;

typedef struct gfx_buffer_desc {
    gfx_buffer_update_mode update_mode;
    gfx_buffer_type type;
    i32 size;
    void *data;
} gfx_buffer_desc;

typedef struct gfx_pipeline_uniform_block{
    const i8* name;
    gfx_buffer_handle buffer;
    bool enabled;
} gfx_pipeline_uniform_block;

typedef struct gfx_pipeline_desc {
    gfx_shader_handle shader;

    gfx_buffer_handle index_buffer;
    gfx_pipeline_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
    gfx_pipeline_uniform_block uniform_blocks[MAXIMUM_PIPELINE_UNIFORM_BLOCKS];

    i8 const *name;
    bool contiguous_buffer;
} gfx_pipeline_desc;

typedef struct gfx_pass_action{
    gfx_color value;
    gfx_pass_action_type action;
} gfx_pass_action;

API gfx_shader_handle gfx_shader_create(gfx_shader_desc const *);
API void gfx_shader_destroy(gfx_shader_handle);

API gfx_buffer_handle gfx_buffer_create(gfx_buffer_desc const *);
API void gfx_buffer_update(gfx_buffer_handle, gfx_buffer_desc const *);
API void gfx_buffer_destroy(gfx_buffer_handle);

API void gfx_begin_default_pass(gfx_pass_action const*);
API void gfx_end_pass();

API void gfx_draw_triangles(i32 start, i32 length);
API void gfx_draw_triangles_indexed(i32 length);

API gfx_pipeline_handle gfx_pipeline_create(gfx_pipeline_desc const *);
API void gfx_apply_pipeline(gfx_pipeline_handle pip);
API void gfx_pipeline_destroy(gfx_pipeline_handle);

#endif //FIXEDPHYSICS_GRAPHICS_H