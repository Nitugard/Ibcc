/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Wire.h"
#include "Allocator.h"
#include "Graphics.h"
#include "GlMath.h"
#include <stddef.h>
#include "Device.h"
#include "./Shaders/Common.h"

typedef struct wire_vertex{
    float pos[3];
    float color[3];
} wire_vertex;

typedef struct wire_list{
    struct wire_vertex* buffer;
    int buffer_count;
    gfx_buffer_handle vertex_buffer;
    gfx_pipeline_handle pip;
    gfx_shader_handle shader;
    uint32_t v_min;
    int32_t projection_uniform;
    int32_t view_uniform;
} wire_list;

typedef struct wire_data {
    wire_list list;
    int32_t capacity;
}wire_data;

void wire_init_buffer(struct wire_list* list, uint32_t size, gfx_shader_handle sh_handle)
{
    list->buffer = OS_MALLOC(sizeof(struct wire_vertex) * size);
    list->buffer_count = 0;

    gfx_buffer_handle buff_v = gfx_buffer_create(GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_DYNAMIC_DRAW, list->buffer, sizeof(struct wire_vertex) * size);
    gfx_pipeline_handle pip = gfx_pipeline_create(sh_handle);

    gfx_pipeline_attr_enable(pip, ATTR_POSITION_NAME, buff_v, 3, offsetof(struct wire_vertex, pos), sizeof(wire_vertex));
    gfx_pipeline_attr_enable(pip, ATTR_COLOR_NAME, buff_v, 3, offsetof(struct wire_vertex, color), sizeof(wire_vertex));

    gfx_pipeline_submit(pip);

    list->vertex_buffer = buff_v;
    list->pip = pip;
    list->shader = sh_handle;
    list->v_min = 0;

    gfx_shader_uniform_enable(list->shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &list->projection_uniform);
    gfx_shader_uniform_enable(list->shader, VIEW_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &list->view_uniform);

}

wire_handle wire_new(int32_t capacity) {

    gfx_shader_handle shader = gfx_shader_create("Gizmos");
    void *vs = device_file_read_text("./Shaders/Gizmos.vs");
    void *fs = device_file_read_text("./Shaders/Gizmos.fs");
    gfx_shader_add_vs(shader, vs);
    gfx_shader_add_fs(shader, fs);
    gfx_shader_submit(shader);
    OS_FREE(vs);
    OS_FREE(fs);

    wire_handle handle = OS_MALLOC(sizeof(struct wire_data));
    handle->capacity = capacity;
    wire_init_buffer(&(handle->list), capacity, shader);

    return handle;
}

void wire_add_vertex(wire_handle handle, float vec[3], float color[3]) {
    wire_vertex *vertex = handle->list.buffer + handle->list.buffer_count;
    os_memcpy(vertex->pos, vec, sizeof(float) * 3);
    os_memcpy(vertex->color, color, sizeof(float) * 3);
    handle->list.buffer_count += 1;
}

void wire_update_buffer(wire_handle handle) {
    gfx_buffer_update(handle->list.vertex_buffer, handle->list.buffer, 0,
                      handle->list.buffer_count * sizeof(struct wire_vertex));
}

void wire_segment_origin(wire_handle handle, float vec[3], float color[3]) {
    float zero[3] = {0, 0, 0};
    wire_segment(handle, vec, zero, color);
}

void wire_segment(wire_handle handle, float vec[3], float origin[3], float color[3])
{
    wire_add_vertex(handle, vec, color);
    wire_add_vertex(handle, origin, color);

    wire_update_buffer(handle);
}

uint64_t wire_get_position(wire_handle handle)
{
    uint32_t c1 = handle->list.buffer_count - handle->list.v_min;
    return c1;
}

void wire_clear(wire_handle handle, uint64_t position) {
    uint32_t c1 = position + handle->list.v_min;
    handle->list.buffer_count = c1;
}

void wire_draw(wire_handle handle, float projection[16], float view[16]) {
    gfx_pipeline_bind(handle->list.pip);
    gfx_shader_uniform_set(handle->list.shader, handle->list.projection_uniform, projection);
    gfx_shader_uniform_set(handle->list.shader, handle->list.view_uniform, view);
    gfx_draw(GFX_LINES, 0, handle->list.buffer_count);
}

void wire_grid(wire_handle handle, uint32_t segments, int mode) {

    int offset = mode * 2;
    int indexes[] = {0, 2, 1, 2, 0, 1};
    int sw[] = {indexes[offset], indexes[offset+1], indexes[offset+1], indexes[offset]};
    int32_t seg_half = segments/2;
    for(int32_t i=0; i<=segments; ++i) {
        int32_t up_limit = seg_half;
        if (i == seg_half)
            up_limit = 0;

        gl_vec3 from = GL_VEC3_ZERO;
        gl_vec3 to = GL_VEC3_ZERO;

        for(int j=0; j<4; j+=2) {

            from.data[sw[j]] = i - seg_half;
            from.data[sw[j + 1]] = up_limit;

            to.data[sw[j]] = i - seg_half;
            to.data[sw[j + 1]] = -seg_half;

            wire_segment(handle,
                         from.data,
                         to.data,
                         gl_vec3_new(0.7, 0.7, 0.7).data);
        }
    }
    wire_segment_origin(handle, gl_vec3_new(100, 0, 0).data, gl_vec3_new(1, 0, 0).data);
    wire_segment_origin(handle, gl_vec3_new(0, 100, 0).data, gl_vec3_new(0, 1, 0).data);
    wire_segment_origin(handle, gl_vec3_new(0, 0, 100).data, gl_vec3_new(0, 0, 1).data);

}

void wire_axis(wire_handle handle, float pos[3]){
    wire_segment(handle, pos, gl_vec3_add(gl_vec3_new(1, 0, 0), gl_vec3_new_arr(pos)).data, gl_vec3_new(1, 0, 0).data);
    wire_segment(handle, pos, gl_vec3_add(gl_vec3_new(0, 1, 0), gl_vec3_new_arr(pos)).data, gl_vec3_new(0, 1, 0).data);
    wire_segment(handle, pos, gl_vec3_add(gl_vec3_new(0, 0, 1), gl_vec3_new_arr(pos)).data, gl_vec3_new(0, 0, 1).data);
}

void wire_apply(wire_handle handle) {
    handle->list.v_min = wire_get_position(handle) + handle->list.v_min;
}

void wire_clear_all(wire_handle handle) {
    wire_clear(handle, 0);
}

void wire_delete(wire_handle handle) {
    gfx_shader_destroy(handle->list.shader);
    gfx_buffer_destroy(handle->list.vertex_buffer);
    gfx_pipeline_destroy(handle->list.pip);
    OS_FREE(handle->list.buffer);
    OS_FREE(handle);
}

