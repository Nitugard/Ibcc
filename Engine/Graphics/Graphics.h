/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_GRAPHICS_H
#define FIXEDPHYSICS_GRAPHICS_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

#define MAXIMUM_PIPELINE_ATTRIBUTES 32
#define MAXIMUM_PIPELINE_UNIFORM_BLOCKS 8
#define MAXIMUM_PIPELINE_UNIFORMS 128

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

typedef enum gfx_draw_type {
    GFX_POINTS = 0x0000,
    GFX_LINES = 0x0001,
    GFX_LINE_LOOP = 0x0002,
    GFX_LINE_STRIP = 0x0003,
    GFX_TRIANGLES = 0x0004,
    GFX_TRIANGLE_STRIP = 0x0005,
    GFX_TRIANGLE_FAN = 0x0006
} gfx_draw_type;

typedef struct gfx_shader *gfx_shader_handle;
typedef struct gfx_pipeline *gfx_pipeline_handle;
typedef struct gfx_buffer *gfx_buffer_handle;
typedef struct gfx_texture *gfx_texture_handle;

typedef struct gfx_color{
    float x,y,z,w;
} gfx_color;

typedef struct gfx_shader_attr {
    int32_t size;
    int32_t num_elements;
} gfx_shader_attr;

typedef struct gfx_pipeline_attr {
    int32_t offset;
    int32_t stride;
    gfx_buffer_handle buffer;
} gfx_pipeline_attr;

typedef struct gfx_shader_stage {
    char const *src;
} gfx_shader_stage;

typedef struct gfx_shader_desc {
    gfx_shader_stage fs, vs;
    char const *name;
    gfx_shader_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];

} gfx_shader_desc;

typedef struct gfx_buffer_desc {
    gfx_buffer_update_mode update_mode;
    gfx_buffer_type type;
    int32_t size;
    void *data;
} gfx_buffer_desc;

typedef struct gfx_texture_desc{
    int32_t width;
    int32_t height;
    int32_t channels;
    void* data;
} gfx_texture_desc;

typedef struct gfx_pipeline_uniform_block{
    const char* name;
    gfx_buffer_handle buffer;
} gfx_pipeline_uniform_block;

typedef struct gfx_pipeline_uniform{
    const char* name;
    //uint32_t size;
    uint32_t offset;
} gfx_pipeline_uniform;

typedef struct gfx_pipeline_desc {
    gfx_shader_handle shader;

    gfx_buffer_handle index_buffer;
    gfx_pipeline_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
    gfx_pipeline_uniform_block uniform_blocks[MAXIMUM_PIPELINE_UNIFORM_BLOCKS];
    gfx_pipeline_uniform uniforms[MAXIMUM_PIPELINE_UNIFORMS];

    char const *name;
} gfx_pipeline_desc;

typedef struct gfx_pass_action{
    gfx_color value;
    gfx_pass_action_type action;
} gfx_pass_action;

API bool gfx_init();
API void gfx_terminate();

API gfx_shader_handle gfx_shader_create(gfx_shader_desc const *);
API void gfx_shader_destroy(gfx_shader_handle);

API gfx_buffer_handle gfx_buffer_create(gfx_buffer_desc const *);
API void gfx_buffer_update(gfx_buffer_handle, gfx_buffer_desc const *);
API void gfx_buffer_destroy(gfx_buffer_handle);

API gfx_texture_handle gfx_texture_create(gfx_texture_desc const*);
API void gfx_texture_bind(gfx_texture_handle);
API void gfx_texture_destroy(gfx_texture_handle);

API void gfx_begin_default_pass(gfx_pass_action const*);
API void gfx_end_pass();

API void gfx_draw(gfx_draw_type type, int32_t start, int32_t length);
API void gfx_draw_id(gfx_draw_type type, int32_t length);


API gfx_pipeline_handle gfx_pipeline_create(gfx_pipeline_desc const *);
API void gfx_pipeline_apply(gfx_pipeline_handle pip);
API void gfx_pipeline_destroy(gfx_pipeline_handle);
API void gfx_pipeline_uniforms_update(gfx_pipeline_handle pip, void* buffer, uint32_t start, int32_t length);

#endif //FIXEDPHYSICS_GRAPHICS_H