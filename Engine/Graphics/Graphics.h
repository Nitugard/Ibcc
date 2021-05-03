/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

#define MAXIMUM_PIPELINE_ATTRIBUTES 32
#define MAXIMUM_PIPELINE_UNIFORM_BLOCKS 8
#define MAXIMUM_PIPELINE_UNIFORMS 128
#define MAXIMUM_FRAMEBUFFER_COLOR_ATTACHMENTS 8

typedef enum gfx_buffer_type {
    GFX_BUFFER_VERTEX,
    GFX_BUFFER_INDEX,
    GFX_BUFFER_UNIFORM,
} gfx_buffer_type;

typedef enum gfx_type{
    GFX_FLOAT1,
    GFX_FLOAT2,
    GFX_FLOAT3,
    GFX_FLOAT4,
    GFX_MAT2,
    GFX_MAT3,
    GFX_MAT4,
    GFX_INT1,
    GFX_INT2,
    GFX_INT3,
    GFX_INT4,
} gfx_type;

typedef enum gfx_blend_type{
    GFX_BLEND_ONE,
    GFX_BLEND_ZERO,
    GFX_BLEND_SRC_ALPHA,
    GFX_BLEND_ONE_MINUS_SRC_ALPHA,
    GFX_BLEND_SRC_COLOR,
    GFX_BLEND_ONE_MINUS_SRC_COLOR,
    GFX_BLEND_DEST_ALPHA,
    GFX_BLEND_ONE_MINUS_DEST_ALPHA,
    GFX_BLEND_DEST_COLOR,
    GFX_BLEND_ONE_MINUS_DEST_COLOR
} gfx_blend_type;

typedef enum gfx_buffer_update_mode {
    GFX_BUFFER_UPDATE_STATIC_DRAW,
    GFX_BUFFER_UPDATE_STATIC_READ,
    GFX_BUFFER_UPDATE_STATIC_COPY,
    GFX_BUFFER_UPDATE_DYNAMIC_DRAW,
    GFX_BUFFER_UPDATE_DYNAMIC_READ,
    GFX_BUFFER_UPDATE_DYNAMIC_COPY,
    GFX_BUFFER_UPDATE_STREAM_DRAW,
    GFX_BUFFER_UPDATE_STREAM_READ,
    GFX_BUFFER_UPDATE_STREAM_COPY,
} gfx_buffer_update_mode;

typedef enum gfx_draw_type {
    GFX_POINTS = 0x0000,
    GFX_LINES = 0x0001,
    GFX_LINE_LOOP = 0x0002,
    GFX_LINE_STRIP = 0x0003,
    GFX_TRIANGLES = 0x0004,
    GFX_TRIANGLE_STRIP = 0x0005,
    GFX_TRIANGLE_FAN = 0x0006
} gfx_draw_type;

typedef enum gfx_texture_type
{
    GFX_TEXTURE_TYPE_SRGB,
    GFX_TEXTURE_TYPE_SRGBA,
    GFX_TEXTURE_TYPE_RGB,
    GFX_TEXTURE_TYPE_RGBA,
    GFX_TEXTURE_TYPE_DEPTH,
    GFX_TEXTURE_TYPE_STENCIL,
    GFX_TEXTURE_TYPE_DEPTH_STENCIL
} gfx_texture_type;

typedef enum gfx_texture_wrap_mode{
    GFX_TEXTURE_WRAP_CLAMP,
    GFX_TEXTURE_WRAP_REPEAT,
    GFX_TEXTURE_WRAP_MIRROR
} gfx_texture_wrap_mode;

typedef enum gfx_texture_filter_mode{
    GFX_TEXTURE_FILTER_NEAREST,
    GFX_TEXTURE_FILTER_LINEAR,
} gfx_texture_filter_mode;

typedef enum gfx_pass_action{
    GFX_PASS_ACTION_NONE = 0x0,
    GFX_PASS_ACTION_CLEAR_COLOR = 0x1,
    GFX_PASS_ACTION_CLEAR_DEPTH = 0x2,
    GFX_PASS_ACTION_CLEAR_STENCIL = 0x4,
} gfx_pass_action;

typedef enum gfx_pass_option{
    GFX_PASS_OPTION_NONE = 0x0,
    GFX_PASS_OPTION_DEPTH_TEST= 0x1,
    GFX_PASS_OPTION_FRAMEBUFFER_SRGB=0x2,
    GFX_PASS_OPTION_CULL_BACK=0x4,
    GFX_PASS_OPTION_CULL_FRONT=0x8,
} gfx_pass_option;

typedef struct gfx_shader *gfx_shader_handle;
typedef struct gfx_pipeline *gfx_pipeline_handle;
typedef struct gfx_buffer *gfx_buffer_handle;
typedef struct gfx_texture *gfx_texture_handle;
typedef struct gfx_framebuffer *gfx_framebuffer_handle;
typedef struct gfx_renderbuffer *gfx_renderbuffer_handle;

typedef struct gfx_color{
    float x,y,z,w;
} gfx_color;

typedef struct gfx_shader_attr {
    int32_t size;
    int32_t num_elements;
} gfx_shader_attr;

typedef struct gfx_pipeline_attr {
    bool enabled;
    int32_t offset;
    int32_t stride;
    gfx_buffer_handle buffer;
    int32_t elements_count;
    int32_t element_size;
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
    gfx_texture_filter_mode filter;
    gfx_texture_type type;
    gfx_texture_wrap_mode wrap;
    int32_t width;
    int32_t height;
    bool mipmaps;
    void* data;
} gfx_texture_desc;

typedef struct gfx_pipeline_uniform_block{
    bool enabled;
    const char* name;
    gfx_buffer_handle buffer;
} gfx_pipeline_uniform_block;

typedef struct gfx_uniform{
    bool enabled;
    void(* setter)(uint32_t id, void* buffer);
    int32_t id;
} gfx_uniform;

typedef struct gfx_pipeline_desc {
    gfx_shader_handle shader;

    gfx_buffer_handle index_buffer;
    gfx_pipeline_attr attrs[MAXIMUM_PIPELINE_ATTRIBUTES];
    gfx_pipeline_uniform_block uniform_blocks[MAXIMUM_PIPELINE_UNIFORM_BLOCKS];

    char const *name;
} gfx_pipeline_desc;

typedef struct gfx_pass_desc {
    gfx_framebuffer_handle fbo_handle;
    gfx_pass_option pass_options;
    gfx_pass_action actions;

    gfx_color clear_color;

} gfx_pass_desc;

typedef struct gfx_framebuffer_attachment{
    bool enabled;
    gfx_texture_handle texture_handle;
} gfx_framebuffer_attachment;

typedef struct gfx_renderbuffer_desc{
    //todo: mostly used as a faster alternative to depth/stencil attachments that are not read from memory as images
} gfx_renderbuffer_desc;

typedef struct gfx_framebuffer_desc {
    gfx_framebuffer_attachment color_attachments[MAXIMUM_FRAMEBUFFER_COLOR_ATTACHMENTS];
    gfx_framebuffer_attachment depth_stencil_attachment;
} gfx_framebuffer_desc;

API bool gfx_init();
API void gfx_terminate();

API gfx_shader_handle gfx_shader_create(gfx_shader_desc const * shader_desc);
API void gfx_shader_destroy(gfx_shader_handle shader);
API void gfx_shader_bind(gfx_shader_handle shader);

API gfx_buffer_handle gfx_buffer_create(gfx_buffer_desc const * buffer_desc);
API void gfx_buffer_update(gfx_buffer_handle, gfx_buffer_desc const * buffer);
API void gfx_buffer_destroy(gfx_buffer_handle buffer);

API gfx_texture_handle gfx_texture_create(gfx_texture_desc const* texture_desc);
API gfx_texture_handle gfx_texture_create_color(gfx_texture_desc const* desc, gfx_color color);
API gfx_texture_type gfx_texture_color_type_from_channels(int32_t channels, bool srgb);
API void gfx_texture_bind(gfx_texture_handle texture, int32_t index);
API void gfx_texture_destroy(gfx_texture_handle texture);

API void gfx_begin_pass(gfx_pass_desc const* desc);
API int32_t gfx_end_pass();

API void gfx_draw(gfx_draw_type type, int32_t start, int32_t length);
API void gfx_draw_id(gfx_draw_type type, int32_t length);

API void gfx_reset_draw_call_count();
API uint32_t gfx_draw_call_count_get();

API void gfx_blend(gfx_blend_type src, gfx_blend_type dest);
API void gfx_blend_enable(bool state);

API void gfx_viewport_set(int32_t width, int32_t height);

API gfx_pipeline_handle gfx_pipeline_create(gfx_pipeline_desc const * pipeline_desc);
API void gfx_pipeline_bind(gfx_pipeline_handle pipeline);
API void gfx_pipeline_destroy(gfx_pipeline_handle pipeline);

API gfx_uniform gfx_uniform_register(gfx_shader_handle shader, const char* name, gfx_type type);
API void gfx_uniform_set(gfx_uniform uniform, void* buffer);

API gfx_framebuffer_handle gfx_framebuffer_create(struct gfx_framebuffer_desc* framebuffer_desc);
API void gfx_framebuffer_destroy(gfx_framebuffer_handle buffer);

#endif //GRAPHICS_H