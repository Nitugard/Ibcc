/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef SOFTMATRIX_H
#define SOFTMATRIX_H

#include "SoftVector.h"


#define SF_MAT_ZERO { .data= {SF_ZERO,SF_ZERO,SF_ZERO,SF_ZERO, SF_ZERO,SF_ZERO,SF_ZERO,SF_ZERO, SF_ZERO,SF_ZERO,SF_ZERO,SF_ZERO, SF_ZERO,SF_ZERO,SF_ZERO,SF_ZERO}}
#define SF_MAT_IDENTITY { .data= {SF_ONE,SF_ZERO,SF_ZERO,SF_ZERO, SF_ZERO,SF_ONE,SF_ZERO,SF_ZERO, SF_ZERO,SF_ZERO,SF_ONE,SF_ZERO, SF_ZERO,SF_ZERO,SF_ZERO,SF_ONE}}

#define SF_MAT_MUL(...) sf_mat_op_cumm_variadic(sf_mat_mul, PP_NARG(__VA_ARGS__), __VA_ARGS__)


typedef struct sf_mat4{

    union{
        sf data[16];
        struct
        {
            sf m00, m01, m02, m03;
            sf m10, m11, m12, m13;
            sf m20, m21, m22, m23;
            sf m30, m31, m32, m33;
        };
    };
} sf_mat4;



API sf_mat4 sf_mat_mul(sf_mat4 const *a, sf_mat4 const *b);
API sf_mat4 sf_mat_mul_p(sf_mat4 const *a, sf b);
API sf_mat4 sf_mat_add(sf_mat4 const *a, sf_mat4 const *b);

API sf_vec4 sf_mat_column_get(sf_mat4 const *a, int32_t column);
API sf_vec4 sf_mat_row_get(sf_mat4 const *a, int32_t row);
API void sf_mat_column_set(sf_vec4 const *a, int32_t column, sf_mat4 *b);
API void sf_mat_row_set(sf_vec4 const *a, int32_t row, sf_mat4 *b);

API sf_mat4 sf_mat_trs(sf_vec3 const *translate, sf_vec3 const *rotate, sf_vec3 const *scale);
API sf_mat4 sf_mat_translate(sf_vec3 const *pos);
API sf_mat4 sf_mat_rotate_x(sf angle);
API sf_mat4 sf_mat_rotate_y(sf angle);
API sf_mat4 sf_mat_rotate_z(sf angle);
API sf_mat4 sf_mat_rotate_zxy(sf_vec3 const *euler_angles);
API sf_mat4 sf_mat_axis_angle(const sf_vec3 *axis, sf angle);
API sf_mat4 sf_mat_perspective(sf fov, sf ar, sf near, sf far);
API sf_mat4 sf_mat_ortographic(sf bot, sf top, sf left, sf right, sf near, sf far);
API sf_mat4 sf_mat_scale(sf_vec3 const *scale);
API sf_mat4 sf_mat_skew(const sf_vec3* a);

API sf sf_mat_det(sf_mat4 const *a);
API sf_mat4 sf_mat_inverse(sf_mat4 const *a);
API sf_mat4 sf_mat_transpose(sf_mat4 const *a);

API sf_mat4 sf_mat_op_cumm(sf_mat4 const *a, sf_mat4 (*op)(sf_mat4 const*, sf_mat4 const*), int32_t count);
API sf_mat4 sf_mat_op_cumm_variadic(sf_mat4(*op)(sf_mat4 const*, sf_mat4 const*), int32_t count, ...);

#endif //SOFTMATRIX_H
