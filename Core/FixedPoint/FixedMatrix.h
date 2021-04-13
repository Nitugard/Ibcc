/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_FIXEDMATRIX_H
#define IBC_FIXEDMATRIX_H

#include "FixedVector.h"

#define FP_MAT_ZERO { .data= {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}}
#define FP_MAT_IDENTITY { .data= {FP_ONE,0,0,0, 0,FP_ONE,0,0, 0,0,FP_ONE,0, 0,0,0,FP_ONE}}

#define FP_MAT_MUL(...) fp_mat_op_cumm_variadic(fp_mat_mul, PP_NARG(__VA_ARGS__), __VA_ARGS__)

struct fp_mat4{

    union{
        fp data[16];
        struct
        {
            fp m00, m01, m02, m03;
            fp m10, m11, m12, m13;
            fp m20, m21, m22, m23;
            fp m30, m31, m32, m33;
        };
    };
};



API fp_mat4 fp_mat_mul(fp_mat4 const *a, fp_mat4 const *b);
API fp_mat4 fp_mat_mul_p(fp_mat4 const *a, fp b);
API fp_mat4 fp_mat_add(fp_mat4 const *a, fp_mat4 const *b);

API fp_vec4 fp_mat_column_get(fp_mat4 const *a, i32 column);
API fp_vec4 fp_mat_row_get(fp_mat4 const *a, i32 row);
API void fp_mat_column_set(fp_vec4 const *a, i32 column, fp_mat4 *b);
API void fp_mat_row_set(fp_vec4 const *a, i32 row, fp_mat4 *b);

API fp_mat4 fp_mat_trs(fp_vec3 const *translate, fp_vec3 const *rotate, fp_vec3 const *scale);
API fp_mat4 fp_mat_translate(fp_vec3 const *pos);
API fp_mat4 fp_mat_rotate_x(fp angle);
API fp_mat4 fp_mat_rotate_y(fp angle);
API fp_mat4 fp_mat_rotate_z(fp angle);
API fp_mat4 fp_mat_rotate_zxy(fp_vec3 const *euler_angles);
API fp_mat4 fp_mat_axis_angle(const fp_vec3 *axis, fp angle);
API fp_mat4 fp_mat_perspective(fp fov, fp ar, fp near, fp far);
API fp_mat4 fp_mat_scale(fp_vec3 const *scale);
API fp_mat4 fp_mat_skew(const fp_vec3* a);

API fp fp_mat_det(fp_mat4 const *a);
API fp_mat4 fp_mat_inverse(fp_mat4 const *a);
API fp_mat4 fp_mat_transpose(fp_mat4 const *a);

API fp_mat4 fp_mat_op_cumm(fp_mat4 const *a, fp_mat4 (*op)(fp_mat4 const*, fp_mat4 const*), i32 count);
API fp_mat4 fp_mat_op_cumm_variadic(fp_mat4(*op)(fp_mat4 const*, fp_mat4 const*), i32 count, ...);


#endif //IBC_FIXEDMATRIX_H
