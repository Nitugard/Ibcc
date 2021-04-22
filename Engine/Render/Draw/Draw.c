/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Draw.h"
#include <Os/Allocator.h>
#include <stddef.h>
#include "Shaders/UnlitVertexColorShader.h"
#include <Containers/Array.h>

#define STARTING_VERTICES 1024

typedef struct dw_vertex{
    float pos[3];
    float color[3];
} dw_vertex;

typedef struct dw_list{
    arr_handle buffer;
    gfx_buffer_handle vertex_buffer;
    gfx_pipeline_handle pip;
    gfx_shader_handle shader;
    uint32_t v_min;
} dw_list;

typedef struct dw_data {
    dw_list list;
}dw_data;

void dw_init_buffer(struct dw_list* list, uint32_t size, gfx_shader_handle sh_handle, gfx_buffer_handle cam_buffer)
{
    list->buffer = arr_new(sizeof(struct dw_vertex), size);
    gfx_buffer_desc buffer_desc = {
            .data = arr_get(list->buffer, 0),
            .size = sizeof(struct dw_vertex) * size,
            .type = VERTEX,
            .update_mode = DYNAMIC_DRAW
    };
    gfx_buffer_handle buff_v = gfx_buffer_create(&buffer_desc);


    gfx_pipeline_desc pip_desc = {
            .shader = sh_handle,
            .index_buffer = 0,
            .uniform_blocks[0] = {
                    .name = "matrices",
                    .buffer = cam_buffer
            },
            .attrs = {
                    [position_attr] = {

                            .buffer = buff_v,
                            .offset = 0,
                            .stride = sizeof(dw_vertex),
                    },
                    [color_attr] = {
                            .buffer = buff_v,
                            .stride = sizeof(dw_vertex),
                            .offset = offsetof(dw_vertex, color),
                    }
            },
    };

    gfx_pipeline_handle pip = gfx_pipeline_create(&pip_desc);
    list->vertex_buffer = buff_v;
    list->pip = pip;
    list->shader = sh_handle;
    list->v_min = 0;
}

dw_handle dw_new(const dw_desc *desc) {

    //create handles
    dw_handle handle = OS_MALLOC(sizeof(struct dw_data));
    gfx_shader_handle sh_handle = gfx_shader_create(&unlit_shader_desc);

    dw_init_buffer(&(handle->list), STARTING_VERTICES, sh_handle, desc->cam_buffer);

    return handle;
}

void dw_set_vertex(dw_handle handle, mm_vec3 vec, mm_vec3 color) {
    dw_vertex *vertex = arr_get(handle->list.buffer, arr_size(handle->list.buffer) );
    arr_resize(handle->list.buffer, arr_size(handle->list.buffer) + 1);
    mm_array_to_float_array(vec.data, 3, vertex->pos);
    mm_array_to_float_array(color.data, 3, vertex->color);
}

void dw_resize_buffer(dw_handle handle)
{

}

void dw_cube(dw_handle handle, mm_vec3 vec, mm_vec3 color, float size)
{
    mm_t half_size = size / 2;
    mm_t half_size_pxp = half_size + vec.x;
    mm_t half_size_pxm = -half_size + vec.x;
    mm_t half_size_pyp = half_size + vec.y;
    mm_t half_size_pym = -half_size + vec.y;
    mm_t half_size_pzp = half_size + vec.z;
    mm_t half_size_pzm = -half_size + vec.z;

    mm_vec3 c1f = mm_vec3_new(half_size_pxm, half_size_pym,half_size_pzm);
    mm_vec3 c2f = mm_vec3_new(half_size_pxp, half_size_pym,half_size_pzm);
    mm_vec3 c3f = mm_vec3_new(half_size_pxm, half_size_pyp,half_size_pzm);
    mm_vec3 c4f = mm_vec3_new(half_size_pxp, half_size_pyp,half_size_pzm);
    mm_vec3 c1b = mm_vec3_new(half_size_pxm, half_size_pym,half_size_pzp);
    mm_vec3 c2b = mm_vec3_new(half_size_pxp, half_size_pym,half_size_pzp);
    mm_vec3 c3b = mm_vec3_new(half_size_pxm, half_size_pyp,half_size_pzp);
    mm_vec3 c4b = mm_vec3_new(half_size_pxp, half_size_pyp,half_size_pzp);

    dw_segment(handle, c2f, c1f, color);
    dw_segment(handle, c3f, c1f, color);
    dw_segment(handle, c2f, c4f, color);
    dw_segment(handle, c3f, c4f, color);

    dw_segment(handle, c2b, c1b, color);
    dw_segment(handle, c3b, c1b, color);
    dw_segment(handle, c2b, c4b, color);
    dw_segment(handle, c3b, c4b, color);

    dw_segment(handle, c1f, c1b, color);
    dw_segment(handle, c2f, c2b, color);
    dw_segment(handle, c3f, c3b, color);
    dw_segment(handle, c4f, c4b, color);
}

void dw_update_buffer(dw_handle handle)
{
    gfx_buffer_desc buffer_desc = {
            .data = arr_get(handle->list.buffer,0),
            .size = arr_max_size(handle->list.buffer) * sizeof(dw_vertex),
            .type = VERTEX,
            .update_mode = STREAM_DRAW
    };
    gfx_buffer_update(handle->list.vertex_buffer, &buffer_desc);
}

void dw_segment_origin(dw_handle handle, mm_vec3 vec, mm_vec3 color) {
    dw_segment(handle, vec, mm_vec3_new(0, 0, 0), color);
}

void dw_segment(dw_handle handle, mm_vec3 vec, mm_vec3 origin, mm_vec3 color)
{
    dw_set_vertex(handle, vec, color);
    dw_set_vertex(handle, origin, color);

    dw_update_buffer(handle);
}

uint64_t dw_get_position(dw_handle handle)
{
    uint32_t c1 = arr_size(handle->list.buffer) - handle->list.v_min;
    return c1;
}

void dw_clear(dw_handle handle, uint64_t position) {
    uint32_t c1 = position + handle->list.v_min;
    arr_resize(handle->list.buffer, c1);
}

void dw_bind(dw_handle handle) {
    gfx_pipeline_apply(handle->list.pip);

    gfx_draw(GFX_LINES, 0, arr_size(handle->list.buffer));

}

void dw_grid(dw_handle handle, uint32_t segments) {

    int32_t seg_half = segments/2;
    for(int32_t i=0; i<=segments; ++i) {
        int32_t up_limit = seg_half;
        if(i == seg_half)
            up_limit = 0;

        dw_segment(handle,
                   mm_vec3_new(i - seg_half, 0, up_limit),
                   mm_vec3_new(i - seg_half, 0, -seg_half),
                   mm_vec3_new(0.5, 0.5, 0.5));
        dw_segment(handle,
                   mm_vec3_new(+up_limit, 0, i - seg_half),
                   mm_vec3_new(-seg_half, 0, i - seg_half),
                   mm_vec3_new(0.5, 0.5, 0.5));
    }

    dw_segment_origin(handle, mm_vec3_new(0, 100, 0), mm_vec3_new(1, 0, 0));
    dw_segment_origin(handle, mm_vec3_new(100, 0, 0), mm_vec3_new(0, 1, 0));
    dw_segment_origin(handle, mm_vec3_new(0, 0, 100), mm_vec3_new(0, 0, 1));

}

void dw_apply(dw_handle handle) {
    handle->list.v_min = dw_get_position(handle) + handle->list.v_min;
}

void dw_clear_all(dw_handle handle) {
    dw_clear(handle, 0);
}

void dw_delete(dw_handle handle) {
    gfx_shader_destroy(handle->list.shader);
    gfx_buffer_destroy(handle->list.vertex_buffer);
    gfx_pipeline_destroy(handle->list.pip);
    arr_delete(handle->list.buffer);
    OS_FREE(handle);
}

