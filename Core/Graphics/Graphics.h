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

#define MAXIMUM_SHADER_ATTRIBUTES 32

typedef enum gfx_buffer_type {
    VERTEX,
    INDEX,
} gfx_buffer_type;

typedef enum gfx_type{
    GFX_FLOAT,
    GFX_INT,
}gfx_type;

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

typedef struct gfx_shader *gfx_shader_handle;
typedef struct gfx_pipeline *gfx_pipeline_handle;
typedef struct gfx_buffer *gfx_buffer_handle;

typedef struct gfx_shader_attr {
    i8 const *name;
    i32 index;
    gfx_type type;
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
    i8 const *entry;
    i32 length;
} gfx_shader_stage;

typedef struct gfx_shader_desc {
    gfx_shader_stage fs, vs;
    i8 const *name;
    gfx_shader_attr attrs[MAXIMUM_SHADER_ATTRIBUTES];

} gfx_shader_desc;

typedef struct gfx_buffer_desc {
    gfx_buffer_update_mode update_mode;
    gfx_buffer_type type;
    i32 size;
    void *data;
} gfx_buffer_desc;


typedef struct gfx_pipeline_desc {
    gfx_shader_handle shader;
    i8 const *name;
    gfx_pipeline_attr attrs[MAXIMUM_SHADER_ATTRIBUTES];
    bool contiguous_buffer;
} gfx_pipeline_desc;


API gfx_shader_handle gfx_shader_create(gfx_shader_desc const *);
API void gfx_shader_destroy(gfx_shader_handle);

API gfx_buffer_handle gfx_buffer_create(gfx_buffer_desc const *);
API void gfx_buffer_update(gfx_buffer_handle, gfx_buffer_desc const *);
API void gfx_buffer_destroy(gfx_buffer_handle);

//see how sokol does it?
API void gfx_clear_command();
API void gfx_draw_command();

API gfx_pipeline_handle gfx_pipeline_create(gfx_pipeline_desc const *);
API void gfx_draw(gfx_pipeline_handle pip);
API void gfx_pipeline_destroy(gfx_pipeline_handle);

#endif //FIXEDPHYSICS_GRAPHICS_H