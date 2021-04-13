/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "FixedVector.h"

#ifndef FP_ASSERT

#include <assert.h>

#define FP_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif


void fp_vec_add(fp const *a, fp const *b, i32 count, fp *out) {
    for (i32 i = 0; i < count; ++i) out[i] = fp_add(a[i], b[i]);
}

void fp_vec_sub(fp const *a, fp const *b, i32 count, fp *out) {
    for (i32 i = 0; i < count; ++i) out[i] = fp_sub(a[i], b[i]);
}

void fp_vec_mul(fp const *a, fp const *b, i32 count, fp *out) {
    for (i32 i = 0; i < count; ++i) out[i] = fp_mul(a[i], b[i]);
}

void fp_vec_div(fp const *a, fp const *b, i32 count, fp *out) {
    for (i32 i = 0; i < count; ++i) out[i] = fp_div(a[i], b[i]);
}


fp fp_vec_dot(fp const *a, fp const *b, i32 count) {
    fp       res = FP_ZERO;
    for (i32 i   = 0; i < count; ++i) res = fp_add(res, fp_mul(a[i], b[i]));
    return res;
}

fp fp_vec_norm_squared(fp const *a, i32 count) {
    return fp_vec_dot(a, a, count);
}

fp fp_vec_norm(fp const *a, i32 count) {
    return fp_sqrt(fp_vec_norm_squared(a, count));
}

void fp_vec_normalize(i32 count, fp *a) {
    fp       norm = fp_vec_norm(a, count);
    fp       rcp  = fp_div(FP_ONE, norm);
    for (i32 i    = 0; i < count; ++i) a[i] = fp_mul(a[i], rcp);
}

fp_vec3 fp_vec3_cross(const fp_vec3 *a, const fp_vec3 *b) {
    fp_vec3 res = {
            .x = fp_sub(fp_mul(a->y, b->z), fp_mul(a->z, b->y)),
            .y = fp_sub(fp_mul(a->z, b->x), fp_mul(a->x, b->z)),
            .z = fp_sub(fp_mul(a->x, b->y), fp_mul(a->y, b->x))
    };
    return res;
}

fp fp_vec3_angle(fp_vec3 const *a, fp_vec3 const *b) {
    return fp_acos(fp_div(fp_vec_dot(a->data, b->data, 3),
                          fp_mul(fp_vec_norm(a->data, 3), fp_vec_norm(b->data, 3))));
}


fp_vec3 fp_vec3_from_float(const f32 *arr) {
    fp_vec3 result;
    for(u32 i=0; i<(sizeof(result.data) / sizeof(fp)); ++i)
        result.data[i] = fp_from_double(arr[i]);
    return result;
}

fp_vec4 fp_vec4_from_float(const f32 *arr) {
    fp_vec4 result;
    for(u32 i=0; i<(sizeof(result.data) / sizeof(fp)); ++i)
        result.data[i] = fp_from_double(arr[i]);
    return result;
}

fp_vec4 fp_vec4_from_int(const i32 *arr) {
    fp_vec4 result;
    for(u32 i=0; i<(sizeof(result.data) / sizeof(fp)); ++i)
        result.data[i] = fp_from_int(arr[i]);
    return result;
}

fp_vec3 fp_vec3_from_int(const i32 *arr) {
    fp_vec3 result;
    for(u32 i=0; i<(sizeof(result.data) / sizeof(fp)); ++i)
        result.data[i] = fp_from_int(arr[i]);
    return result;
}
