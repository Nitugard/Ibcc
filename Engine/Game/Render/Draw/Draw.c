/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Draw.h"
#include <Os/Allocator.h>
#include <Graphics/Graphics.h>
#include "../Shaders/UnlitColor.h"


typedef struct dw_data{
    float* buffer;
    gfx_buffer_handle vertex_buffer;
    gfx_pipeline_handle pip;
    gfx_shader_handle  shader;
    uint32_t vertices;
    uint32_t max_vertices;
} dw_data;

#define VERTEX_STRIDE 6
#define VERTEX_STRIDE_B (sizeof(float) * VERTEX_STRIDE)

dw_handle dw_new(const dw_desc *desc) {

    //create handles
    dw_handle handle = OS_MALLOC(sizeof(struct dw_data));
    handle->buffer = OS_MALLOC(VERTEX_STRIDE_B * MAXIMUM_VERTICES);

    gfx_shader_handle sh_handle = gfx_shader_create(&unlit_shader_desc);
    gfx_buffer_desc buffer_desc = {
            .data = handle->buffer,
            .size = VERTEX_STRIDE_B * MAXIMUM_VERTICES,
            .type = VERTEX,
            .update_mode = STREAM_DRAW
    };
    gfx_buffer_handle buff_v = gfx_buffer_create(&buffer_desc);


    gfx_pipeline_desc pip_desc = {
            .shader = sh_handle,
            .index_buffer = 0,
            .uniform_blocks[0] = {
                    .enabled = true,
                    .name = "matrices",
                    .buffer = desc->cam_buffer
            },
            .contiguous_buffer = false,
            .attrs = {
                [position_attr] = {

                    .buffer = buff_v,
                    .enabled = true,
                    .offset = 0,
                    .stride = VERTEX_STRIDE_B
                },
                [color_attr] = {
                        .buffer = buff_v,
                        .enabled = true,
                        .offset = sizeof(float) * 3,
                        .stride = VERTEX_STRIDE_B,
                }
            }
    };
    gfx_pipeline_handle pip = gfx_pipeline_create(&pip_desc);
    handle->vertex_buffer = buff_v;
    handle->pip = pip;
    handle->shader = sh_handle;
    handle->max_vertices = MAXIMUM_VERTICES;
    handle->vertices = 0;
    return handle;
}

void dw_set_vertex(dw_handle handle, mm_vec3 vec, mm_vec3 color) {
    mm_array_to_float_array(vec.data, 3, handle->buffer + handle->vertices * VERTEX_STRIDE);
    mm_array_to_float_array(color.data, 3, handle->buffer + handle->vertices * VERTEX_STRIDE + 3);
    handle->vertices++;
}

void dw_vector(dw_handle handle, mm_vec3 vec, mm_vec3 color) {
    dw_vector_origin(handle, vec, mm_vec3_new(0, 0, 0), color);
}

void dw_vector_origin(dw_handle handle, mm_vec3 vec, mm_vec3 origin, mm_vec3 color)
{
    dw_set_vertex(handle, vec, color);
    dw_set_vertex(handle, origin, color);

    gfx_buffer_desc buffer_desc = {
            .data = handle->buffer,
            .size = handle->max_vertices * VERTEX_STRIDE_B,
            .type = VERTEX,
            .update_mode = STREAM_DRAW
    };
    gfx_buffer_update(handle->vertex_buffer, &buffer_desc);
}


void dw_bind(dw_handle handle) {
    gfx_apply_pipeline(handle->pip);
    gfx_draw_lines(0, handle->vertices);
}

