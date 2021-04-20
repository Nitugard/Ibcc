/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Math/Math.h>

#ifndef API
#define API
#endif

typedef struct controller_camera_init_desc{
    mm_vec3 pos;
    mm_vec3 rot_euler;
    float movement_speed;
    double rotation_speed;
    bool first_person;
} controller_camera_init_desc;

typedef struct controller_camera_data* controller_camera_handle;

API controller_camera_handle controller_camera_new(controller_camera_init_desc const* desc);
API mm_mat4 controller_camera_get_mat(controller_camera_handle handle);
API void controller_camera_update(controller_camera_handle handle, float dt);
API void controller_camera_destroy(controller_camera_handle handle);

#endif //CONTROLLER_H
