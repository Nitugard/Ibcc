/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "SoftVector.h"

#ifndef SF_ASSERT
#include <assert.h>
#define SF_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif


sf_vec3 sf_vec3_new(sf x, sf y, sf z) {
    sf_vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

sf_vec4 sf_vec4_new(sf x, sf y, sf z, sf w) {
    sf_vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

void sf_vec_add(sf const *a, sf const *b, i32 count, sf *out) {
    for (i32 i = 0; i < count; ++i) out[i] = sf_add(a[i], b[i]);
}

void sf_vec_sub(sf const *a, sf const *b, i32 count, sf *out) {
    for (i32 i = 0; i < count; ++i) out[i] = sf_sub(a[i], b[i]);
}

void sf_vec_mul(sf const *a, sf const *b, i32 count, sf *out) {
    for (i32 i = 0; i < count; ++i) out[i] = sf_mul(a[i], b[i]);
}

void sf_vec_div(sf const *a, sf const *b, i32 count, sf *out) {
    for (i32 i = 0; i < count; ++i) out[i] = sf_div(a[i], b[i]);
}


sf sf_vec_dot(sf const *a, sf const *b, i32 count) {
    sf res = SF_ZERO;
    for (i32 i = 0; i < count; ++i) res = sf_add(res, sf_mul(a[i], b[i]));
    return res;
}

sf sf_vec_norm_squared(sf const *a, i32 count) {
    return sf_vec_dot(a, a, count);
}

sf sf_vec_norm(sf const *a, i32 count) {
    return sf_sqrt(sf_vec_norm_squared(a, count));
}

void sf_vec_normalize(i32 count, sf *a) {
    sf norm = sf_vec_norm(a, count);
    sf rcp = sf_div(SF_ONE, norm);
    for (i32 i = 0; i < count; ++i) a[i] = sf_mul(a[i], rcp);
}

sf_vec3 sf_vec3_cross(const sf_vec3 *a, const sf_vec3 *b) {
    sf_vec3 res = {
            .x = sf_sub(sf_mul(a->y, b->z), sf_mul(a->z, b->y)),
            .y = sf_sub(sf_mul(a->z, b->x), sf_mul(a->x, b->z)),
            .z = sf_sub(sf_mul(a->x, b->y), sf_mul(a->y, b->x))
    };
    return res;
}

sf sf_vec3_angle(sf_vec3 const *a, sf_vec3 const *b) {
    return sf_acos(sf_div(sf_vec_dot(a->data, b->data, 3),
                          sf_mul(sf_vec_norm(a->data, 3), sf_vec_norm(b->data, 3))));
}

