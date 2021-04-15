/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef SOFTVECTOR_H
#define SOFTVECTOR_H

#include "SoftFloat.h"

#define FP_VEC3_ZERO { .data= SF_ZERO,SF_ZERO,SF_ZERO} }
#define FP_VEC3_ONE { .data= {FP_ONE,FP_ONE,FP_ONE} }
#define FP_VEC3_LEFT { .data= {FP_NEG_ONE,SF_ZERO,SF_ZERO} }
#define FP_VEC3_RIGHT { .data= {FP_ONE,SF_ZERO,SF_ZERO} }
#define FP_VEC3_UP { .data= {SF_ZERO,FP_ONE,SF_ZERO} }
#define FP_VEC3_DOWN { .data= {SF_ZERO,FP_NEG_ONE,SF_ZERO} }
#define FP_VEC3_FWD { .data= {SF_ZERO,SF_ZERO,FP_ONE} }
#define FP_VEC3_BACK { .data= {SF_ZERO,SF_ZERO,FP_NEG_ONE} }

typedef struct sf_vec3{
    union {
        sf data[3];
        struct {
            sf x, y, z;
        };
    };
} sf_vec3;

typedef struct sf_vec4{
    union {
        sf data[4];
        struct {
            sf x, y, z, w;
        };
    };
} sf_vec4;

/*
 * Initializers
 */

API sf_vec3 sf_vec3_new(sf x, sf y, sf z);
API sf_vec4 sf_vec4_new(sf x, sf y, sf z, sf w);


/*
 * Operators
 */

API void sf_vec_add(sf const *a, sf const *b, int32_t count, sf *out);
API void sf_vec_sub(sf const *a, sf const *b, int32_t count, sf *out);
API void sf_vec_mul(sf const *a, sf const *b, int32_t count, sf *out);
API void sf_vec_div(sf const *a, sf const *b, int32_t count, sf *out);

/*
 * Functions
 */

API sf sf_vec_dot(sf const *a, sf const *b, int32_t count);
API sf sf_vec_norm_squared(sf const *a, int32_t count);
API sf sf_vec_norm(sf const *a, int32_t count);
API void sf_vec_normalize(int32_t count, sf *a);

API sf_vec3 sf_vec3_cross(const sf_vec3 *a, const sf_vec3 *b);
API sf sf_vec3_angle(sf_vec3 const *a, sf_vec3 const *b);


#endif //SOFTVECTOR_H

#ifdef SOFTVECTOR_IMPLEMENTATION

#ifndef CORE_ASSERT
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
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

void sf_vec_add(sf const *a, sf const *b, int32_t count, sf *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = sf_add(a[i], b[i]);
}

void sf_vec_sub(sf const *a, sf const *b, int32_t count, sf *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = sf_sub(a[i], b[i]);
}

void sf_vec_mul(sf const *a, sf const *b, int32_t count, sf *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = sf_mul(a[i], b[i]);
}

void sf_vec_div(sf const *a, sf const *b, int32_t count, sf *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = sf_div(a[i], b[i]);
}


sf sf_vec_dot(sf const *a, sf const *b, int32_t count) {
    sf res = SF_ZERO;
    for (int32_t i = 0; i < count; ++i) res = sf_add(res, sf_mul(a[i], b[i]));
    return res;
}

sf sf_vec_norm_squared(sf const *a, int32_t count) {
    return sf_vec_dot(a, a, count);
}

sf sf_vec_norm(sf const *a, int32_t count) {
    return sf_sqrt(sf_vec_norm_squared(a, count));
}

void sf_vec_normalize(int32_t count, sf *a) {
    sf norm = sf_vec_norm(a, count);
    sf rcp = sf_div(SF_ONE, norm);
    for (int32_t i = 0; i < count; ++i) a[i] = sf_mul(a[i], rcp);
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

#undef SOFTVECTOR_IMPLEMENTATION
#endif
