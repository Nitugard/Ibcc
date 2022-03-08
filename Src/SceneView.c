/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "SceneView.h"
#include "Allocator.h"
#include "Graphics.h"
#include "Scene.h"
#include "Controller.h"
#include "GlMath.h"
#include "Wire.h"
#include "Device.h"

#define DEFAULT_DISTANCE 8
#define DEFAULT_MOVE_SPEED 20
#define DEFAULT_ROTATE_SPEED 20
#define DELTA_EPSILON 1E-4

typedef struct scene_view_controller{
    gl_mat projection;
    float fov;
    float znear, zfar;

    float jaw, pitch;
    float move_speed, rot_speed;

    gl_mat rotation;
    gl_vec3 position;

    gl_vec3 offset;
    float distance;
} scene_view_controller;

typedef struct scene_view{
    gfx_framebuffer_handle fbo;
    gfx_texture_handle color_tex;
    gfx_texture_handle depth_tex;
    int32_t width, height;
    wire_handle wire;
    scene_view_controller controller;
    char const* name;
    bool dirty;
    enum scene_view_type view_type;
} scene_view;

float color[4] = {0.24, 0.24, 0.24, 1};

void scene_view_controller_update_projection(scene_view_handle handle) {

    float ar = (float) handle->width / handle->height;

    if (handle->view_type == SCENE_VIEW_PERSPECTIVE) {

        handle->controller.projection = gl_mat_perspective(handle->controller.fov, ar, handle->controller.znear,
                                                           handle->controller.zfar);
    } else {
        float fov = handle->controller.fov / 16;
        handle->controller.projection = gl_mat_ortographic(-fov, fov, -fov * ar, fov * ar, handle->controller.znear,
                                                           handle->controller.zfar);
    }
}

void scene_view_controller_update_internal(scene_view_handle handle) {

    gl_mat mat_pitch = gl_mat_rotate_y(-handle->controller.pitch);
    gl_mat mat_jaw = gl_mat_rotate_x(handle->controller.jaw);
    handle->controller.rotation = gl_mat_mul(mat_pitch, mat_jaw);

    gl_vec3 rel_pos = gl_mat_mul_vec(handle->controller.rotation, gl_vec3_new(0, 0, handle->controller.distance));
    handle->controller.position = gl_vec3_add(handle->controller.offset, rel_pos);
}

void create_fbo(scene_view* handle, int32_t width, int32_t height) {

    handle->color_tex = gfx_texture_create(width, height, 0, GFX_TEXTURE_TYPE_RGB, GFX_TEXTURE_FILTER_LINEAR,
                                           GFX_TEXTURE_WRAP_CLAMP);
    handle->depth_tex = gfx_texture_create(width, height, 0, GFX_TEXTURE_TYPE_DEPTH, GFX_TEXTURE_FILTER_LINEAR,
                                           GFX_TEXTURE_WRAP_CLAMP);

    handle->fbo = gfx_framebuffer_create(handle->color_tex, handle->depth_tex);
}

const char* view_type_to_string(scene_view_type view_type) {
    switch (view_type) {

        case SCENE_VIEW_PERSPECTIVE:
            return "SCENE_VIEW_PERSPECTIVE";
        case SCENE_VIEW_ORTOGRAPHIC_FRONT:
            return "SCENE_VIEW_ORTOGRAPHIC_FRONT";
    }
    return "DEFAULT";
}

scene_view_handle scene_view_create(int32_t width, int32_t height, scene_view_type view_type)
{
    scene_view_handle handle = OS_MALLOC(sizeof(struct scene_view));
    os_memset(handle, 0, sizeof(struct scene_view));

    handle->controller.fov = 60;
    handle->controller.zfar = 1000;
    handle->controller.znear = 0.01;
    handle->view_type = view_type;
    handle->name = view_type_to_string(view_type);
    handle->width = width;
    handle->height = height;
    handle->dirty = true;
    handle->wire = wire_new(512);

    create_fbo(handle, width, height);

    switch (view_type) {

        case SCENE_VIEW_PERSPECTIVE: {
            wire_grid(handle->wire, 32, 0);
        }
            break;
        case SCENE_VIEW_ORTOGRAPHIC_FRONT: {
            wire_grid(handle->wire, 32, 2);
        }
            break;
    }

    wire_apply(handle->wire);

    handle->controller.rotation = gl_mat_new_identity();
    handle->controller.distance = DEFAULT_DISTANCE;
    handle->controller.move_speed = DEFAULT_MOVE_SPEED;
    handle->controller.rot_speed = DEFAULT_ROTATE_SPEED;

    scene_view_controller_update_internal(handle);
    scene_view_controller_update_projection(handle);
    return handle;
}

void scene_view_update_controller(scene_view_handle handle) {

    device_joystick joystick = device_joystick_get();
    float dt = device_dt_get();
    bool has_changed_position = false;
    bool has_changed_projection = false;

    if (gl_abs(joystick.pointer.dy) > DELTA_EPSILON ||
        gl_abs(joystick.pointer.dx) > DELTA_EPSILON) {

        if (joystick.mouse.rmb_press) {

            float new_jaw = (handle->controller.jaw + joystick.pointer.dy * dt * handle->controller.rot_speed);
            float new_pitch = (handle->controller.pitch + joystick.pointer.dx * dt * handle->controller.rot_speed);

            handle->controller.jaw = new_jaw;
            handle->controller.pitch = new_pitch;
            has_changed_position = true;
        }

        if (joystick.mouse.mmb_press) {
            gl_vec3 move = gl_vec3_new(-joystick.pointer.dx * dt,
                                       joystick.pointer.dy * dt, 0);
            handle->controller.offset = gl_vec3_add(handle->controller.offset,
                                                    gl_mat_mul_vec(handle->controller.rotation, move));
            has_changed_position = true;
        }
    }

    if (gl_abs(joystick.pointer.scroll_y) > DELTA_EPSILON) {
        if (handle->view_type == SCENE_VIEW_PERSPECTIVE) {
            handle->controller.distance += joystick.pointer.scroll_y * dt * handle->controller.move_speed;
            if (handle->controller.distance < DELTA_EPSILON) {
                gl_vec3 move = gl_vec3_new(0, 0, joystick.pointer.scroll_y * dt * handle->controller.move_speed);
                handle->controller.offset = gl_vec3_add(handle->controller.offset,
                                                        gl_mat_mul_vec(handle->controller.rotation, move));

                handle->controller.distance = DELTA_EPSILON;
            }

            has_changed_position = true;
        }
        else {
            handle->controller.fov += joystick.pointer.scroll_y * dt * handle->controller.move_speed * 3;
            if (handle->controller.fov < DELTA_EPSILON) {
                handle->controller.fov = DELTA_EPSILON;
            }
            has_changed_projection = true;
        }
    }

    if(has_changed_position)
    {
        scene_view_controller_update_internal(handle);
        handle->dirty = true;
    }

    if(has_changed_projection){
        scene_view_controller_update_projection(handle);
        handle->dirty = true;
    }
}

void scene_view_render(scene_view_handle handle,void* scene) {
    if (handle->dirty) {
        gfx_begin_pass(handle->fbo, GFX_PASS_OPTION_DEPTH_TEST | GFX_PASS_OPTION_CULL_BACK,
                       GFX_PASS_ACTION_CLEAR_DEPTH | GFX_PASS_ACTION_CLEAR_COLOR,
                        color);

        wire_clear_all(handle->wire);
        gfx_viewport_set(handle->width, handle->height);
        wire_axis(handle->wire, handle->controller.offset.data);

        gl_mat tr = handle->controller.rotation;
        tr = gl_mat_set_translation(tr, handle->controller.position);

        wire_draw(handle->wire, handle->controller.projection.data, gl_mat_inverse(tr).data);
        scene_draw_with_camera(scene, handle->controller.projection.data, tr.data, handle->view_type == SCENE_VIEW_PERSPECTIVE);
        gfx_end_pass();

        handle->dirty = false;
    }
}

void scene_view_render_get(scene_view_handle handle, int32_t* color, int32_t* depth) {
    if (handle->color_tex != 0 ) {
        if(color != 0)
            *color = gfx_texture_get_id(handle->color_tex);
    }else {
        if(color != 0)
            *color = -1;
    }
    if (handle->depth_tex != 0) {
        if(depth != 0)
            *depth = gfx_texture_get_id(handle->depth_tex);
    }
    else {
        if(depth != 0)
            *depth = -1;
    }
}

void scene_view_resize(scene_view_handle handle, int32_t w, int32_t h) {
    if (handle->width != w && handle->height != h) {

        handle->width = w;
        handle->height = h;
        handle->dirty = true;

        if(w != 0 && h != 0) {
            gfx_texture_destroy(handle->color_tex);
            gfx_texture_destroy(handle->depth_tex);
            gfx_framebuffer_destroy(handle->fbo);
            create_fbo(handle, w, h);
            scene_view_controller_update_projection(handle);
        }

    }
}

void scene_view_get_size(scene_view_handle handle, int32_t* width, int32_t* height){
    *width = handle->width;
    *height = handle->height;
}

void scene_view_destroy(scene_view_handle handle){
    gfx_texture_destroy(handle->depth_tex);
    gfx_texture_destroy(handle->color_tex);
    gfx_framebuffer_destroy(handle->fbo);
    OS_FREE(handle);
}

char const* scene_view_get_name(scene_view_handle handle){
    return handle->name;
}

void scene_view_flag_dirty(scene_view_handle handle) {
    handle->dirty = true;
}
