/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Sprite.h"
#include <Graphics/Graphics.h>
#include <Os/Log.h>
#include <Os/Platform.h>
#include <Os/Allocator.h>
#include <Math/GlMath.h>
#include <Containers/Array.h>
#include <Shaders/SpriteShader.h>
#include <Device/Device.h>

#include <stddef.h>

#define MAXIMUM_VERTICES 6

typedef struct sprite_vertex{
    gl_vec2 pos;
    gl_vec2 uv;
} sprite_vertex;

typedef struct sprite_quad{
    sprite_vertex vertices[MAXIMUM_VERTICES];
} sprite_quad;

typedef struct sprite_data{
    sprite_desc desc;
    gl_mat projection;
    int32_t vertices;
} sprite_data;


typedef struct sprite_manager{
    void* data;
    gfx_shader_handle shader;
    gfx_buffer_handle vbo;
    arr_handle sprites_arr;
    gfx_pipeline_handle pipeline;
    int32_t capacity;
    gfx_uniform projection_uniform;
}sprite_manager;

gfx_shader_handle sprite_shader_handle;
sprite_manager manager;

sprite_data* sprite_get(int32_t index){
    return (sprite_data*)arr_get(manager.sprites_arr, index);
}

void sprite_init(int32_t capacity) {
    os_memset(&manager, 0, sizeof(struct sprite_manager));
    manager.shader = gfx_shader_create(&sprite_shader_desc);
    manager.sprites_arr = arr_new(sizeof(struct sprite_data), capacity);
    manager.capacity = capacity;

    gfx_buffer_desc vbo_desc = {
            .update_mode = GFX_BUFFER_UPDATE_DYNAMIC_DRAW,
            .type = GFX_BUFFER_VERTEX,
            .size = sizeof(struct sprite_quad) * capacity,
            .data = OS_MALLOC(sizeof(struct sprite_quad) * capacity)
    };
    manager.vbo = gfx_buffer_create(&vbo_desc);

    gfx_pipeline_desc pipeline = {
            .name = "sprite_pipeline",
            .shader = manager.shader,
            .attrs = {
                    [ATTR_POSITION_LOCATION] = {
                            .enabled = true,
                            .buffer = manager.vbo,
                            .stride = sizeof(struct sprite_vertex),
                            .offset = offsetof(sprite_vertex, pos),
                            .elements_count = 2,
                            .element_size = sizeof(float)
                    },
                    [ATTR_UV_LOCATION] = {
                            .enabled = true,
                            .buffer = manager.vbo,
                            .stride = sizeof(struct sprite_vertex),
                            .offset = offsetof(sprite_vertex, uv),
                            .elements_count = 2,
                            .element_size = sizeof(float)
                    },

            }
    };
    manager.data = vbo_desc.data;
    manager.pipeline = gfx_pipeline_create(&pipeline);
    manager.projection_uniform = gfx_uniform_register(manager.shader, "projection", GFX_MAT4);
}

void sprite_terminate() {

    while (arr_size(manager.sprites_arr) > 0) {
        sprite_delete(sprite_get(0));
    }

    gfx_buffer_destroy(manager.vbo);
    gfx_shader_destroy(manager.shader);
    gfx_pipeline_destroy(manager.pipeline);
    OS_FREE(manager.data);
    arr_delete(manager.sprites_arr);
}

sprite_handle sprite_new(sprite_desc desc) {
    if (arr_full(manager.sprites_arr)) {
        LOG_ERROR("Sprites array is full\n");
        return 0;
    }

    sprite_data temp_data = {0};
    arr_add(manager.sprites_arr, &temp_data);
    sprite_vertex v0 = {.pos = {.x = 0, .y = 0}, .uv = {.x = 0, .y = 0}};
    sprite_vertex v1 = {.pos = {.x = desc.width, .y = 0}, .uv = {.x = 1, .y = 0}};
    sprite_vertex v2 = {.pos = {.x = 0, .y = desc.height}, .uv = {.x = 0, .y = 1}};
    sprite_vertex v3 = {.pos = {.x = desc.width, .y = desc.height}, .uv = {.x = 1, .y = 1}};

    sprite_quad quad = {
            .vertices = {v0, v1, v3, v0, v3, v2}
    };
    int32_t index = arr_size(manager.sprites_arr) - 1;
    os_memcpy((char *) manager.data + index * sizeof(struct sprite_quad), quad.vertices, sizeof(struct sprite_quad));
    sprite_data *data = arr_get(manager.sprites_arr, index);
    sprite_update(data, desc);
    gfx_buffer_desc vbo_desc = {
            .update_mode = GFX_BUFFER_UPDATE_DYNAMIC_DRAW,
            .type = GFX_BUFFER_VERTEX,
            .size = sizeof(struct sprite_quad) * manager.capacity,
            .data = manager.data,
    };
    data->vertices = 6;
    gfx_buffer_update(manager.vbo, &vbo_desc);
    return data;
}

void sprite_update(sprite_handle handle, sprite_desc desc) {
    int32_t width, height;
    device_window_dimensions_get(&width, &height);


    gl_mat orto = gl_mat_ortographic(-desc.offset_y * desc.scale,
                                     (height - desc.offset_y) * desc.scale,
                                     -desc.offset_x * desc.scale, (width - desc.offset_x) * desc.scale,
                                     -1,
                                     1);
    handle->projection = orto;
    handle->desc = desc;
}

void sprite_delete(sprite_handle handle) {
    int32_t size = arr_size(manager.sprites_arr);
    bool removed = false;
    for (int32_t i = 0; i < size; ++i) {
        if (sprite_get(i) == handle) {
            removed = true;
            arr_remove_swap(manager.sprites_arr, i);
            break;
        }
    }

    assert(removed);
}

void sprite_draw() {
    int32_t size = arr_size(manager.sprites_arr);
    gfx_pipeline_bind(manager.pipeline);
    for (int32_t i = 0; i < size; ++i) {
        sprite_data *data = sprite_get(i);
        if (data->desc.custom_shader.enabled)
            gfx_shader_bind(data->desc.custom_shader.gfx_shader_handle);
        else
            gfx_shader_bind(manager.shader);
        gfx_uniform_set(manager.projection_uniform, data->projection.data);
        if (data->desc.gfx_texture_handle != 0)
            gfx_texture_bind(data->desc.gfx_texture_handle, 0);
        gfx_draw(GFX_TRIANGLES, i * 6, data->vertices);
    }
}

