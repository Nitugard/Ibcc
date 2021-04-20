/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Controller.h"
#include <Os/Allocator.h>
#include <Device/Device.h>

typedef struct controller_camera_data{
    mm_vec3 pos;
    mm_vec3 rot_euler;
    mm_mat4 projection;
    mm_mat4 view;
    controller_camera_init_desc init_desc;
} controller_camera_data;

void controller_camera_fp_update(controller_camera_handle handle, float dt) {

    mm_vec3 up = MM_VEC3_UP;
    mm_vec3 fwd = MM_VEC3_FWD;

    //translation
    mm_vec3 delta = MM_VEC3_ZERO;
    if (device_events_get_key(DEVICE_KEY_W) == DEVICE_PRESS_ACTION)
        delta.z = mm_add(delta.z, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_A) == DEVICE_PRESS_ACTION)
        delta.x = mm_add(delta.x, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_S) == DEVICE_PRESS_ACTION)
        delta.z = mm_sub(delta.z, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_D) == DEVICE_PRESS_ACTION)
        delta.x = mm_sub(delta.x, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_SPACE) == DEVICE_PRESS_ACTION)
        delta.y = mm_add(delta.y, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_LEFT_SHIFT) == DEVICE_PRESS_ACTION)
        delta.y = mm_sub(delta.y, MM_ONE);
    if (device_events_get_key(DEVICE_KEY_E) == DEVICE_PRESS_ACTION)
        handle->rot_euler.y = mm_sub(handle->rot_euler.y, MM_MUL(mm_new(1), dt, handle->init_desc.rotation_speed));
    if (device_events_get_key(DEVICE_KEY_Q) == DEVICE_PRESS_ACTION)
        handle->rot_euler.y = mm_add(handle->rot_euler.y, MM_MUL(mm_new(1), dt, handle->init_desc.rotation_speed));
    if (device_events_get_key(DEVICE_KEY_R) == DEVICE_PRESS_ACTION)
        handle->rot_euler.x = mm_sub(handle->rot_euler.x, MM_MUL(mm_new(1), dt, handle->init_desc.rotation_speed));
    if (device_events_get_key(DEVICE_KEY_F) == DEVICE_PRESS_ACTION)
        handle->rot_euler.x = mm_add(handle->rot_euler.x, MM_MUL(mm_new(1), dt, handle->init_desc.rotation_speed));


    //rotation
    device_mouse_state state = device_events_get_mouse();
    handle->rot_euler.x = mm_sub(handle->rot_euler.x, MM_MUL(state.dy, dt, handle->init_desc.rotation_speed));
    handle->rot_euler.y = mm_sub(handle->rot_euler.y, MM_MUL(state.dx, dt, handle->init_desc.rotation_speed));
    handle->rot_euler.z = MM_ZERO;

    //clamp
    mm_t clamp = mm_new(80);
    if (mm_less(clamp, handle->rot_euler.x))
        handle->rot_euler.x = clamp;
    else if (mm_less(handle->rot_euler.x, mm_negate(clamp)))
        handle->rot_euler.x = mm_negate(clamp);


    mm_mat4 roty = mm_mat_rotate_y(handle->rot_euler.y);
    mm_mat4 rotx = mm_mat_rotate_x(handle->rot_euler.x);
    mm_mat4 rot = mm_mat_mul(&roty, &rotx);
    mm_vec3 target = mm_mat_mul_vec(&rot, &fwd);
    mm_vec3 center = mm_vec3_add(handle->pos, target);
    handle->view = mm_mat_look_at(center, handle->pos, up);

    delta = mm_mat_mul_vec(&rot, &delta);
    delta = mm_vec3_mul(delta, mm_vec3_new_scalar(mm_mul(handle->init_desc.movement_speed, dt)));
    handle->pos = mm_vec3_add(delta, handle->pos);
}

controller_camera_handle controller_camera_new(const controller_camera_init_desc *desc) {
    controller_camera_handle handle = OS_MALLOC(sizeof(struct controller_camera_data));
    handle->init_desc = *desc;
    handle->pos = desc->pos;
    handle->rot_euler = desc->rot_euler;
    handle->projection = mm_mat_perspective(mm_new(60), mm_new_fraction(800, 600), mm_new_fraction(1, 10), mm_new(100));
    return handle;
}

void controller_camera_update(controller_camera_handle handle, float dt) {
    if(handle->init_desc.first_person){
        controller_camera_fp_update(handle, dt);
    }
}

void controller_camera_update_mvp(controller_camera_handle handle, float* proj, float* view, float* cam_pos) {
    mm_array_to_float_array(handle->projection.data, 16, proj);
    mm_array_to_float_array(handle->view.data, 16, view);
    mm_array_to_float_array(handle->pos.data, 3, cam_pos);
}

void controller_camera_destroy(controller_camera_handle handle) {
    OS_FREE(handle);
}
