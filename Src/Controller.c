/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Controller.h"
#include "GlMath.h"
#include "Allocator.h"
#include "Device.h"

#ifndef CORE_ASSERT
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif

#define DELTA_EPSILON 1E-4

typedef struct controller_data{
    float jaw, pitch;
    float move_speed, rot_speed;

    gl_mat rotation;
    gl_vec3 position;

    gl_vec3 offset;
    float distance;
} controller_data;
