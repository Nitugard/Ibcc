/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_GRAPHICS_H
#define IBCWEB_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

#define LOG_BUFFER_SIZE 1024

#define MAXIMUM_DRAW_PASSES_PER_FRAME 32

#define MAXIMUM_SHADER_NAME_LENGTH 128
#define MAXIMUM_PIPELINES_PER_SHADER 1024

#define MAXIMUM_ATTRIBUTE_NAME_LENGTH 32
#define MAXIMUM_UNIFORM_NAME_LENGTH 32

#define MAXIMUM_ATTRIBUTES_PER_SHADER 8
#define MAXIMUM_UNIFORMS_PER_SHADER 32

#define MAXIMUM_ATTRIBUTE_COMMANDS_PER_PIPELINE 8
#define MAXIMUM_UNIFORM_COMMANDS_PER_SHADER 32

#define MAXIMUM_PATH_LENGTH 1024

#ifndef IBC_API
#define IBC_API
#endif

typedef enum gfx_buffer_type {
    GFX_BUFFER_VERTEX,
    GFX_BUFFER_INDEX,
} gfx_buffer_type;

typedef enum gfx_type{
    GFX_TYPE_FLOAT_VEC_1,
    GFX_TYPE_FLOAT_VEC_2,
    GFX_TYPE_FLOAT_VEC_3,
    GFX_TYPE_FLOAT_VEC_4,
    GFX_TYPE_INTEGER_VEC_1,
    GFX_TYPE_INTEGER_VEC_2,
    GFX_TYPE_INTEGER_VEC_3,
    GFX_TYPE_INTEGER_VEC_4,
    GFX_TYPE_FLOAT_MAT_3,
    GFX_TYPE_FLOAT_MAT_4,
    GFX_TYPE_SAMPLER_CUBE,
    GFX_TYPE_TEXTURE,
    GFX_TYPE_INVALID,
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
    GFX_TEXTURE_TYPE_RGB16,
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
    GFX_PASS_OPTION_CULL_BACK=0x4,
    GFX_PASS_OPTION_CULL_FRONT=0x8,
} gfx_pass_option;

typedef enum gfx_fbo_render_type{
    GFX_RENDER_TEXTURE,
    GFX_RENDER_CUBE_MAP,
} gfx_fbo_render_type;

typedef enum gfx_resource_status{
    GFX_RESOURCE_DESTROYED,
    GFX_RESOURCE_CREATED,
    GFX_RESOURCE_ACTIVE,
    GFX_RESOURCE_INVALID
} gfx_resource_status;

typedef struct gfx_shader *gfx_shader_handle;
typedef struct gfx_pipeline *gfx_pipeline_handle;
typedef struct gfx_buffer *gfx_buffer_handle;
typedef struct gfx_texture *gfx_texture_handle;
typedef struct gfx_texture_cubemap *gfx_texture_cubemap_handle;
typedef struct gfx_framebuffer *gfx_framebuffer_handle;
typedef struct gfx_renderbuffer *gfx_renderbuffer_handle;

typedef void(*shader_change_callback)(gfx_shader_handle handle);
typedef void(*gfx_log_callback)(const char* log, bool error);

IBC_API bool gfx_init();
IBC_API void gfx_terminate();
IBC_API void gfx_log_callback_set(gfx_log_callback callback);

IBC_API gfx_shader_handle gfx_shader_create(const char* name);
IBC_API void gfx_shader_add_vs(gfx_shader_handle handle, const char* vs);
IBC_API void gfx_shader_add_fs(gfx_shader_handle handle, const char* fs);
IBC_API void gfx_shader_submit(gfx_shader_handle handle);
IBC_API void gfx_shader_destroy(gfx_shader_handle handle);
IBC_API void gfx_shader_reload(gfx_shader_handle handle);
IBC_API void gfx_shader_status_change_callback_set(shader_change_callback callback);
IBC_API gfx_resource_status gfx_shader_status(gfx_shader_handle handle);
IBC_API void gfx_shader_fs_get(gfx_shader_handle handle, char ** ptr);
IBC_API void gfx_shader_vs_get(gfx_shader_handle handle, char ** ptr);
IBC_API void gfx_shader_name_get(gfx_shader_handle handle, char ** ptr);

IBC_API gfx_buffer_handle gfx_buffer_create(enum gfx_buffer_type type, enum gfx_buffer_update_mode mode, void* data, int32_t size);
IBC_API void gfx_buffer_update(gfx_buffer_handle handle, void* data, int32_t offset, int32_t size);
IBC_API void gfx_buffer_destroy(gfx_buffer_handle handle);
IBC_API gfx_resource_status gfx_buffer_status(gfx_buffer_handle handle);

IBC_API gfx_texture_handle gfx_texture_load(const char* path, enum gfx_texture_type type, enum gfx_texture_filter_mode filter, enum gfx_texture_wrap_mode wrap);
IBC_API gfx_texture_handle gfx_texture_create(int32_t width, int32_t height, void* data, enum gfx_texture_type type, enum gfx_texture_filter_mode filter, enum gfx_texture_wrap_mode wrap);
IBC_API int32_t gfx_texture_get_id(gfx_texture_handle handle);
IBC_API void gfx_texture_bind(gfx_texture_handle handle, int32_t slot);
IBC_API void gfx_texture_destroy(gfx_texture_handle handle);
IBC_API gfx_resource_status gfx_texture_status(gfx_texture_handle handle);

IBC_API gfx_texture_cubemap_handle gfx_texture_cubemap_create(const char* path, const char* names[6], enum gfx_texture_type type, enum gfx_texture_filter_mode filter);
IBC_API void gfx_texture_cubemap_bind(gfx_texture_cubemap_handle handle);
IBC_API void gfx_texture_cubemap_destroy(gfx_texture_cubemap_handle handle);

IBC_API void gfx_begin_pass(gfx_framebuffer_handle handle, enum gfx_pass_option option, enum gfx_pass_action action, float color[4]);
IBC_API int32_t gfx_end_pass();

IBC_API void gfx_viewport_set(int32_t width, int32_t height);

IBC_API void gfx_draw(enum gfx_draw_type type, int32_t start, int32_t length);
IBC_API void gfx_draw_id(enum gfx_draw_type type, int32_t length);

IBC_API void gfx_blend(enum gfx_blend_type src, enum gfx_blend_type dest);
IBC_API void gfx_blend_enable(bool state);

IBC_API gfx_pipeline_handle gfx_pipeline_create(gfx_shader_handle handle);
IBC_API void gfx_pipeline_index_enable(gfx_pipeline_handle handle, gfx_buffer_handle buffer);
IBC_API void gfx_pipeline_attr_enable(gfx_pipeline_handle handle, const char* name, gfx_buffer_handle buffer, int32_t count, int32_t offset, int32_t stride);
IBC_API void gfx_pipeline_submit(gfx_pipeline_handle handle);
IBC_API void gfx_pipeline_bind(gfx_pipeline_handle handle);
IBC_API void gfx_pipeline_reload(gfx_pipeline_handle handle);
IBC_API void gfx_pipeline_destroy(gfx_pipeline_handle handle);
IBC_API void gfx_pipeline_use_depth_buffer(gfx_pipeline_handle handle, bool value);

IBC_API void gfx_shader_uniform_enable(gfx_shader_handle handle, const char* name, gfx_type type, int32_t* uniform_index);
IBC_API void gfx_shader_uniform_set(gfx_shader_handle handle, uint32_t uniform_index, void* data);

IBC_API gfx_framebuffer_handle gfx_framebuffer_create(gfx_texture_handle color_texture, gfx_texture_handle depth_texture);
IBC_API void gfx_framebuffer_destroy(gfx_framebuffer_handle handle);

#endif //IBCWEB_GRAPHICS_H
