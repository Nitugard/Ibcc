/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_FIXEDVECTOR_H
#define IBC_FIXEDVECTOR_H

#include "FixedPoint.h"

#define FP_VEC3_ZERO { .data= 0,0,0} }
#define FP_VEC3_ONE { .data= {FP_ONE,FP_ONE,FP_ONE} }
#define FP_VEC3_LEFT { .data= {FP_NEG_ONE,0,0} }
#define FP_VEC3_RIGHT { .data= {FP_ONE,0,0} }
#define FP_VEC3_UP { .data= {0,FP_ONE,0} }
#define FP_VEC3_DOWN { .data= {0,FP_NEG_ONE,0} }
#define FP_VEC3_FWD { .data= {0,0,FP_ONE} }
#define FP_VEC3_BACK { .data= {0,0,FP_NEG_ONE} }

struct fp_vec3{
    union {
        fp data[3];
        struct {
            fp x, y, z;
        };
    };
};

struct fp_vec4{
    union {
        fp data[4];
        struct {
            fp x, y, z, w;
        };
    };
};

/*
 * Initializers
 */

API fp_vec3 fp_vec3_from_float(f32 const* arr);
API fp_vec4 fp_vec4_from_float(f32 const* arr);
API fp_vec4 fp_vec4_from_int(i32 const* arr);
API fp_vec3 fp_vec3_from_int(i32 const* arr);

/*
 * Operators
 */

API void fp_vec_add(fp const *a, fp const *b, i32 count, fp *out);
API void fp_vec_sub(fp const *a, fp const *b, i32 count, fp *out);
API void fp_vec_mul(fp const *a, fp const *b, i32 count, fp *out);
API void fp_vec_div(fp const *a, fp const *b, i32 count, fp *out);

/*
 * Functions
 */

API fp fp_vec_dot(fp const *a, fp const *b, i32 count);
API fp fp_vec_norm_squared(fp const *a, i32 count);
API fp fp_vec_norm(fp const *a, i32 count);
API void fp_vec_normalize(i32 count, fp *a);

API fp_vec3 fp_vec3_cross(const fp_vec3 *a, const fp_vec3 *b);
API fp fp_vec3_angle(fp_vec3 const *a, fp_vec3 const *b);




#endif //IBC_FIXEDVECTOR_H
