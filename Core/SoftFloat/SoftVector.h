/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SOFTVECTOR_H
#define IBC_SOFTVECTOR_H

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

API void sf_vec_add(sf const *a, sf const *b, i32 count, sf *out);
API void sf_vec_sub(sf const *a, sf const *b, i32 count, sf *out);
API void sf_vec_mul(sf const *a, sf const *b, i32 count, sf *out);
API void sf_vec_div(sf const *a, sf const *b, i32 count, sf *out);

/*
 * Functions
 */

API sf sf_vec_dot(sf const *a, sf const *b, i32 count);
API sf sf_vec_norm_squared(sf const *a, i32 count);
API sf sf_vec_norm(sf const *a, i32 count);
API void sf_vec_normalize(i32 count, sf *a);

API sf_vec3 sf_vec3_cross(const sf_vec3 *a, const sf_vec3 *b);
API sf sf_vec3_angle(sf_vec3 const *a, sf_vec3 const *b);


#endif //IBC_SOFTVECTOR_H
