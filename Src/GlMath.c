/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "GlMath.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef CORE_ASSERT
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif

gl_t gl_abs(gl_t a) { if(a < 0) return -a; return a; }
gl_t gl_sqrt(gl_t a) { return sqrt(a); }
int32_t gl_sign(gl_t a) { if (a >= 0) return 1; return -1; }
int32_t gl_sign_diff(gl_t a, gl_t b) { return gl_sign(a) * gl_sign(b); }
gl_t gl_min(gl_t a, gl_t b) { if (a > b) return b; return a; }
gl_t gl_max(gl_t a, gl_t b) { if (a > b) return a; return b; }
gl_t gl_deg2rad(gl_t a) { return a * GL_RCP_180 * GL_PI; }
gl_t gl_rad2deg(gl_t a){ return a * 180 * GL_RCP_PI; }
gl_t gl_round(gl_t a) { return round(a); }
gl_t gl_floor(gl_t a) { return floor(a); }
gl_t gl_ceil(gl_t a) { return ceil(a); }


int32_t gl_compare(gl_t a, gl_t b) { if(a > b) return 1; if(a<b) return -1; return 0;}
bool gl_less_equal(gl_t a, gl_t b) { return a <= b; }
bool gl_less(gl_t a, gl_t b) { return a < b;}
bool gl_equal(gl_t a, gl_t b) { return a == b;}


gl_t gl_sin(gl_t a) { return sin(a); }
gl_t gl_cos(gl_t a) { return cos(a); }
gl_t gl_tan(gl_t a) { return tan(a); }

gl_t gl_asin(gl_t a) { return asin(a); }
gl_t gl_acos(gl_t a) { return acos(a); }
gl_t gl_atan(gl_t a) { return atan(a); }
gl_t gl_atan2(gl_t a, gl_t b) { return atan2(a, b); }


gl_t gl_array_op_cumm(gl_t const *a, gl_t (*op)(gl_t, gl_t), int32_t count) {
    gl_t res = a[0];
    for (int32_t i = 1; i < count; ++i) res = op(res, a[i]);
    return res;
}

void gl_array_op(gl_t (*op)(gl_t), int32_t count, gl_t *array) {
    for (int32_t i = 0; i < count; ++i) array[i] = op(array[i]);
}

gl_t gl_array_op_cugl_variadic(gl_t(*op)(gl_t, gl_t), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    gl_t num = (float)va_arg(args, double);
    for (int32_t i = 1; i < count; ++i)
        num = op(num, (float)va_arg(args, double));
    va_end(args);
    return num;
}

void gl_array_op_variadic(gl_t (*op)(gl_t), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    for (int32_t i = 1; i < count; ++i) {
        gl_t *num = va_arg(args, gl_t*);
        *num = op(*num);
    }
    va_end(args);
}



gl_vec3 gl_vec3_new(gl_t x, gl_t y, gl_t z) {
    gl_vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

API gl_vec3 gl_vec3_new_scalar(gl_t x)
{
    gl_vec3 result;
    result.x = x;
    result.y = x;
    result.z = x;
    return result;
}

gl_vec4 gl_vec4_new(gl_t x, gl_t y, gl_t z, gl_t w) {
    gl_vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

void gl_vec_add(gl_t const *a, gl_t const *b, int32_t count, gl_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = (a[i] + b[i]);
}

void gl_vec_sub(gl_t const *a, gl_t const *b, int32_t count, gl_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = (a[i] - b[i]);
}

void gl_vec_mul(gl_t const *a, gl_t const *b, int32_t count, gl_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = (a[i] * b[i]);
}

void gl_vec_div(gl_t const *a, gl_t const *b, int32_t count, gl_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = (a[i] / b[i]);
}


gl_t gl_vec_dot(gl_t const *a, gl_t const *b, int32_t count) {
    gl_t res = 0;
    for (int32_t i = 0; i < count; ++i) res = res + a[i] * b[i];
    return res;
}

gl_t gl_vec_norm_squared(gl_t const *a, int32_t count) {
    return gl_vec_dot(a, a, count);
}

gl_t gl_vec_norm(gl_t const *a, int32_t count) {
    return gl_sqrt(gl_vec_norm_squared(a, count));
}

void gl_vec_normalize(gl_t *a, int32_t count) {
    gl_t norm = gl_vec_norm(a, count);
    gl_t rcp = 1.0f / norm;
    for (int32_t i = 0; i < count; ++i) a[i] = a[i]*rcp;
}

void gl_vec_negate(gl_t *a, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) a[i] = -(a[i]);
}

gl_vec3 gl_vec3_normalize(gl_vec3 a) {
    gl_vec_normalize(a.data, 3);
    return a;
}

gl_t gl_vec3_norm(gl_vec3 a){
    return gl_vec_norm(a.data, 3);
}

gl_vec3 gl_vec3_norm_squared(gl_vec3 a){
    gl_vec_norm_squared(a.data, 3);
    return a;
}

gl_t gl_vec3_dot(gl_vec3 a, gl_vec3 b)
{
    return gl_vec_dot(a.data, b.data, 3);
}

gl_vec3 gl_vec3_add(gl_vec3 a, gl_vec3 b){ gl_vec3 r; gl_vec_add(a.data, b.data, 3, r.data); return r; }
gl_vec3 gl_vec3_sub(gl_vec3 a, gl_vec3 b){ gl_vec3 r; gl_vec_sub(a.data, b.data, 3, r.data); return r; }
gl_vec3 gl_vec3_mul(gl_vec3 a, gl_vec3 b){ gl_vec3 r; gl_vec_mul(a.data, b.data, 3, r.data); return r; }
gl_vec3 gl_vec3_div(gl_vec3 a, gl_vec3 b){ gl_vec3 r; gl_vec_div(a.data, b.data, 3, r.data); return r; }


gl_vec3 gl_vec3_cross(gl_vec3 a, gl_vec3 b) {
    gl_vec3 res = {
            .x = ((a.y * b.z) - (a.z * b.y)),
            .y = ((a.z * b.x) - (a.x * b.z)),
            .z = ((a.x * b.y) - (a.y * b.x))
    };
    return res;
}

gl_t gl_vec3_angle(gl_vec3 a, gl_vec3 b) {
    return gl_acos((gl_vec_dot(a.data, b.data, 3) /
                          (gl_vec_norm(a.data, 3) * gl_vec_norm(b.data, 3))));
}

gl_mat gl_mat_translate(gl_vec3 pos) {
    gl_mat result = GL_MAT_IDENTITY;
    result.m03 = pos.x;
    result.m13 = pos.y;
    result.m23 = pos.z;
    return result;
}

gl_mat gl_mat_remove_translation(gl_mat m) {
    m.m03 = 0;
    m.m13 = 0;
    m.m23 = 0;
    return m;
}


gl_mat gl_mat_set_translation(gl_mat m, struct gl_vec3 v) {
    m.m03 = v.data[0];
    m.m13 = v.data[1];
    m.m23 = v.data[2];
    return m;
}


gl_vec3 gl_mat_get_translation(gl_mat mat){
    return gl_vec3_new(mat.m03, mat.m13, mat.m23);
}

gl_mat gl_mat_skew(gl_vec3 a) {
    gl_mat result = {.data =
            {
                    0, -(a.z), a.y,
                    a.z, 0, -(a.x),
                    -(a.y), a.x, 0
            }
    };
    return result;
}

gl_mat gl_mat_axis_angle(gl_vec3 axis, gl_t angle) {
    const gl_t s = gl_sin(gl_deg2rad(angle));
    const gl_t c = gl_cos(gl_deg2rad(angle));
    const gl_t t = 1.0f- c;
    gl_vec3 naxis = axis;
    gl_vec_normalize(naxis.data, 3);
    const gl_mat skew = gl_mat_skew(naxis);
    const gl_mat skew2 = gl_mat_mul(skew, skew);
    const gl_mat id = GL_MAT_IDENTITY;
    const gl_mat term1 = gl_mat_mul_p(skew, s);
    const gl_mat term2 = gl_mat_mul_p(skew2, t);
    const gl_mat term3 = gl_mat_add(term1, term2);
    return gl_mat_add(id, term3);
}

gl_mat gl_mat_scale(gl_vec3 scale) {
    gl_mat result = GL_MAT_IDENTITY;
    result.m00 = scale.x;
    result.m11 = scale.y;
    result.m22 = scale.z;
    return result;

}

gl_mat gl_mat_mul(gl_mat a, gl_mat b) {
    gl_mat  result;
    for (int32_t i = 0; i < 4; ++i) {
        for (int32_t j = 0; j < 4; ++j) {
            result.data[i * 4 + j] = gl_vec_dot(gl_mat_row_get(&a, i).data, gl_mat_column_get(&b, j).data, 4);
        }
    }

    return result;
}

void gl_mat_column_set(gl_vec4 const *a, int32_t column, gl_mat *b) {
    for (int32_t i = 0; i < 4; ++i)
        b->data[column + i * 4] = a->data[i];
}

gl_vec4 gl_mat_column_get(gl_mat const *a, int32_t column) {
    gl_vec4  result;
    for (int32_t i = 0; i < 4; ++i)
        result.data[i] = a->data[column + i * 4];
    return result;
}

void gl_mat_row_set(gl_vec4 const *a, int32_t row, gl_mat *b) {
    for (int32_t i = 0; i < 4; ++i)
        b->data[(row * 4) + i] = a->data[i];
}

gl_vec4 gl_mat_row_get(gl_mat const *a, int32_t row) {
    gl_vec4  result;
    for (int32_t i = 0; i < 4; ++i)
        result.data[i] = a->data[(row * 4) + i];
    return result;
}

gl_mat gl_mat_add(gl_mat a, gl_mat b) {
    gl_mat  result;
    for (int32_t i = 0; i < 16; ++i)
        result.data[i] = (a.data[i] + b.data[i]);
    return result;
}

gl_mat gl_mat_mul_p(gl_mat a, gl_t b) {
    gl_mat result;
    for(int32_t i=0; i < 16; ++i)
        result.data[i] = (a.data[i] * b);
    return result;
}

API gl_vec3 gl_mat_mul_vec(gl_mat a, gl_vec3 b)
{
    gl_vec4 res;
    gl_vec4 v4 = {.x = b.x, .y = b.y, .z = b.z, .w = 1};
    gl_vec4 temp;
    for(int32_t i=0; i < 4; ++i)
    {
        temp = gl_mat_row_get(&a, i);
        res.data[i] = gl_vec_dot(temp.data, v4.data, 4);
    }
    return gl_vec3_new(res.x, res.y, res.z);
}

gl_mat gl_mat_trs(gl_vec3 translate, gl_vec3 rotate, gl_vec3 scale) {
    const gl_mat sm = gl_mat_scale(scale);
    const gl_mat tm = gl_mat_translate(translate);
    const gl_mat rm = gl_mat_rotate_zxy(rotate);
    const gl_mat rs = gl_mat_mul(rm, sm);
    const gl_mat trs = gl_mat_mul(tm, rs);
    return trs;
}

gl_mat gl_mat_rotate_x(gl_t angle) {
    angle = gl_deg2rad(angle);
    const gl_t c = gl_cos(angle);
    const gl_t s = gl_sin(angle);
    gl_mat result = {.data = {
            1.0f,	0,	0, 0,
            0,	c, -(s), 0,
            0,	s,	c, 0,
            0,  0,  0,1.0f
    }};
    return result;
}

gl_mat gl_mat_rotate_y(gl_t angle) {
    angle = gl_deg2rad(angle);
    const gl_t c = gl_cos(angle);
    const gl_t s = gl_sin(angle);
    gl_mat result = {.data ={
            c, 0, s, 0,
            0, 1.0f, 0, 0,
            -(s), 0, c, 0,
            0, 0, 0,1.0f
    }};

    return result;
}

gl_mat gl_mat_rotate_z(gl_t angle) {
    angle = gl_deg2rad(angle);
    const gl_t c = gl_cos(angle);
    const gl_t s = gl_sin(angle);
    gl_mat result = {.data ={
            c, -(s), 0, 0,
            s, c, 0, 0,
            0, 0, 1.0f, 0,
            0, 0, 0, 1.0f,
    }};
    return result;
}

gl_mat gl_mat_rotate_zxy(gl_vec3 euler_angles) {
    const gl_mat rz = gl_mat_rotate_z(euler_angles.z);
    const gl_mat rx = gl_mat_rotate_x(euler_angles.x);
    const gl_mat ry = gl_mat_rotate_y(euler_angles.y);
    const gl_mat zx = gl_mat_mul(rx, rz);
    const gl_mat zxy = gl_mat_mul(ry, zx);
    return zxy;
}

gl_mat gl_mat_perspective(gl_t fov, gl_t ar, gl_t near, gl_t far) {
    gl_t ctg = 1.0f / gl_tan(fov * (GL_PI / 360.0f));
    gl_mat res = GL_MAT_IDENTITY;
    res.m00 = ctg / ar;
    res.m11 = ctg;
    res.m32 = -1.0f;
    res.m22 = (near + far) / (near - far);
    res.m23 = (2.0f * near * far) / (near - far);
    res.m33 = 0.0f;
    return res;
}

gl_mat gl_mat_transpose(gl_mat a) {
    struct gl_mat result;
    for(int32_t c = 0; c < 4; ++c) {
        for (int32_t r = 0; r < 4; ++r) {
            result.data[c + r * 4] = a.data[r + c * 4];
        }
    }

    return result;
}

gl_mat gl_mat_op_cumm(gl_mat* a, gl_mat (*op)(gl_mat, gl_mat), int32_t count) {
    gl_mat res = a[0];
    for (int32_t i = 1; i < count; ++i) res = op(res, a[i]);
    return res;
}

gl_mat gl_mat_op_cumm_variadic(gl_mat(*op)(gl_mat, gl_mat), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    gl_mat num = va_arg(args, gl_mat);
    for (int32_t i = 1; i < count; ++i) {
        num = op(num, va_arg(args, gl_mat));
    }
    va_end(args);
    return num;
}

gl_mat gl_mat_ortographic(gl_t bot, gl_t top, gl_t left, gl_t right, gl_t near, gl_t far) {

    gl_t two = 2;
    gl_mat result = GL_MAT_IDENTITY;
    result.m00 = (two / (right - left));
    result.m11 = (two / (top - bot));
    result.m22 = (two / (near - far));
    result.m33 = 1.0f;
    result.m03 = ((left + right) / (left - right));
    result.m13 = ((bot + top) / (bot - top));
    result.m23 = ((far + near) / (near - far));
    return result;
}

gl_mat gl_mat_look_at(gl_vec3 forward, gl_vec3 eye, gl_vec3 up) {
    gl_vec3 zaxis = gl_vec3_normalize(forward);
    gl_vec3 xaxis = gl_vec3_normalize(gl_vec3_cross(zaxis, up));
    gl_vec3 yaxis = gl_vec3_cross(xaxis, zaxis);

    gl_vec_negate(zaxis.data, 3);

    gl_mat viewMatrix = {.rows = {
            gl_vec4_new(xaxis.x, xaxis.y, xaxis.z, -gl_vec3_dot(xaxis, eye)),
            gl_vec4_new(yaxis.x, yaxis.y, yaxis.z, -gl_vec3_dot(yaxis, eye)),
            gl_vec4_new(zaxis.x, zaxis.y, zaxis.z, -gl_vec3_dot(zaxis, eye)),
            gl_vec4_new(0, 0, 0, 1)
    }
    };

    return viewMatrix;
}

gl_vec3 gl_vec3_new_arr(float *arr) {
    gl_vec3 result;
    result.x =arr[0];
    result.y =arr[1];
    result.z =arr[2];
    return result;
}

gl_vec4 gl_vec4_new_arr(float *arr) {
    gl_vec4 result;
    result.x = arr[0];
    result.y = arr[1];
    result.z = arr[2];
    result.w = arr[3];
    return result;
}

gl_mat gl_mat_new() {
    gl_mat result = GL_MAT_ZERO;
    return result;
}

gl_mat gl_mat_new_identity() {
    gl_mat result = GL_MAT_IDENTITY;
    return result;
}

gl_mat gl_mat_new_array(float *arr) {
    gl_mat result;
    for(int32_t i=0; i<16; ++i) result.data[i] = arr[i];
    return result;
}

gl_mat gl_mat_inverse(gl_mat m1) {
    gl_t det = gl_mat_det(m1);
    assert(det != 0);
    gl_t m00 = m1.m12 * m1.m23 * m1.m31 - m1.m13 * m1.m22 * m1.m31 + m1.m13 * m1.m21 * m1.m32 -
               m1.m11 * m1.m23 * m1.m32 - m1.m12 * m1.m21 * m1.m33 + m1.m11 * m1.m22 * m1.m33;
    gl_t m01 = m1.m03 * m1.m22 * m1.m31 - m1.m02 * m1.m23 * m1.m31 - m1.m03 * m1.m21 * m1.m32 +
               m1.m01 * m1.m23 * m1.m32 + m1.m02 * m1.m21 * m1.m33 - m1.m01 * m1.m22 * m1.m33;
    gl_t m02 = m1.m02 * m1.m13 * m1.m31 - m1.m03 * m1.m12 * m1.m31 + m1.m03 * m1.m11 * m1.m32 -
               m1.m01 * m1.m13 * m1.m32 - m1.m02 * m1.m11 * m1.m33 + m1.m01 * m1.m12 * m1.m33;
    gl_t m03 = m1.m03 * m1.m12 * m1.m21 - m1.m02 * m1.m13 * m1.m21 - m1.m03 * m1.m11 * m1.m22 +
               m1.m01 * m1.m13 * m1.m22 + m1.m02 * m1.m11 * m1.m23 - m1.m01 * m1.m12 * m1.m23;
    gl_t m10 = m1.m13 * m1.m22 * m1.m30 - m1.m12 * m1.m23 * m1.m30 - m1.m13 * m1.m20 * m1.m32 +
               m1.m10 * m1.m23 * m1.m32 + m1.m12 * m1.m20 * m1.m33 - m1.m10 * m1.m22 * m1.m33;
    gl_t m11 = m1.m02 * m1.m23 * m1.m30 - m1.m03 * m1.m22 * m1.m30 + m1.m03 * m1.m20 * m1.m32 -
               m1.m00 * m1.m23 * m1.m32 - m1.m02 * m1.m20 * m1.m33 + m1.m00 * m1.m22 * m1.m33;
    gl_t m12 = m1.m03 * m1.m12 * m1.m30 - m1.m02 * m1.m13 * m1.m30 - m1.m03 * m1.m10 * m1.m32 +
               m1.m00 * m1.m13 * m1.m32 + m1.m02 * m1.m10 * m1.m33 - m1.m00 * m1.m12 * m1.m33;
    gl_t m13 = m1.m02 * m1.m13 * m1.m20 - m1.m03 * m1.m12 * m1.m20 + m1.m03 * m1.m10 * m1.m22 -
               m1.m00 * m1.m13 * m1.m22 - m1.m02 * m1.m10 * m1.m23 + m1.m00 * m1.m12 * m1.m23;
    gl_t m20 = m1.m11 * m1.m23 * m1.m30 - m1.m13 * m1.m21 * m1.m30 + m1.m13 * m1.m20 * m1.m31 -
               m1.m10 * m1.m23 * m1.m31 - m1.m11 * m1.m20 * m1.m33 + m1.m10 * m1.m21 * m1.m33;
    gl_t m21 = m1.m03 * m1.m21 * m1.m30 - m1.m01 * m1.m23 * m1.m30 - m1.m03 * m1.m20 * m1.m31 +
               m1.m00 * m1.m23 * m1.m31 + m1.m01 * m1.m20 * m1.m33 - m1.m00 * m1.m21 * m1.m33;
    gl_t m22 = m1.m01 * m1.m13 * m1.m30 - m1.m03 * m1.m11 * m1.m30 + m1.m03 * m1.m10 * m1.m31 -
               m1.m00 * m1.m13 * m1.m31 - m1.m01 * m1.m10 * m1.m33 + m1.m00 * m1.m11 * m1.m33;
    gl_t m23 = m1.m03 * m1.m11 * m1.m20 - m1.m01 * m1.m13 * m1.m20 - m1.m03 * m1.m10 * m1.m21 +
               m1.m00 * m1.m13 * m1.m21 + m1.m01 * m1.m10 * m1.m23 - m1.m00 * m1.m11 * m1.m23;
    gl_t m30 = m1.m12 * m1.m21 * m1.m30 - m1.m11 * m1.m22 * m1.m30 - m1.m12 * m1.m20 * m1.m31 +
               m1.m10 * m1.m22 * m1.m31 + m1.m11 * m1.m20 * m1.m32 - m1.m10 * m1.m21 * m1.m32;
    gl_t m31 = m1.m01 * m1.m22 * m1.m30 - m1.m02 * m1.m21 * m1.m30 + m1.m02 * m1.m20 * m1.m31 -
               m1.m00 * m1.m22 * m1.m31 - m1.m01 * m1.m20 * m1.m32 + m1.m00 * m1.m21 * m1.m32;
    gl_t m32 = m1.m02 * m1.m11 * m1.m30 - m1.m01 * m1.m12 * m1.m30 - m1.m02 * m1.m10 * m1.m31 +
               m1.m00 * m1.m12 * m1.m31 + m1.m01 * m1.m10 * m1.m32 - m1.m00 * m1.m11 * m1.m32;
    gl_t m33 = m1.m01 * m1.m12 * m1.m20 - m1.m02 * m1.m11 * m1.m20 + m1.m02 * m1.m10 * m1.m21 -
               m1.m00 * m1.m12 * m1.m21 - m1.m01 * m1.m10 * m1.m22 + m1.m00 * m1.m11 * m1.m22;
    gl_mat res = {
            .data = {
                    m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33
            }
    };
    gl_t rcp_det = 1.0 / det;
    for (int32_t i = 0; i < 16; ++i) res.data[i] = res.data[i] * rcp_det;

    return res;
}

gl_t gl_mat_det(gl_mat a) {
    return a.m03 * a.m12 * a.m21 * a.m30 - a.m02 * a.m13 * a.m21 * a.m30 - a.m03 * a.m11 * a.m22 * a.m30 +
           a.m01 * a.m13 * a.m22 * a.m30 +
           a.m02 * a.m11 * a.m23 * a.m30 - a.m01 * a.m12 * a.m23 * a.m30 - a.m03 * a.m12 * a.m20 * a.m31 +
           a.m02 * a.m13 * a.m20 * a.m31 +
           a.m03 * a.m10 * a.m22 * a.m31 - a.m00 * a.m13 * a.m22 * a.m31 - a.m02 * a.m10 * a.m23 * a.m31 +
           a.m00 * a.m12 * a.m23 * a.m31 +
           a.m03 * a.m11 * a.m20 * a.m32 - a.m01 * a.m13 * a.m20 * a.m32 - a.m03 * a.m10 * a.m21 * a.m32 +
           a.m00 * a.m13 * a.m21 * a.m32 +
           a.m01 * a.m10 * a.m23 * a.m32 - a.m00 * a.m11 * a.m23 * a.m32 - a.m02 * a.m11 * a.m20 * a.m33 +
           a.m01 * a.m12 * a.m20 * a.m33 +
           a.m02 * a.m10 * a.m21 * a.m33 - a.m00 * a.m12 * a.m21 * a.m33 - a.m01 * a.m10 * a.m22 * a.m33 +
           a.m00 * a.m11 * a.m22 * a.m33;

}

gl_t gl_clamp(gl_t a, gl_t min, gl_t max) {
    return gl_max(min, gl_min(max, a));
}

gl_mat gl_mat_from_quaternion(gl_vec4 q) {
    //taken from:
    //https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    gl_mat result = GL_MAT_IDENTITY;
    gl_t sqw = q.w * q.w;
    gl_t sqx = q.x * q.x;
    gl_t sqy = q.y * q.y;
    gl_t sqz = q.z * q.z;

    gl_t invs = 1.0f / (sqx + sqy + sqz + sqw);
    result.m00 = (sqx - sqy - sqz + sqw) * invs;
    result.m11 = (-sqx + sqy - sqz + sqw) * invs;
    result.m22 = (-sqx - sqy + sqz + sqw) * invs;

    gl_t tmp1 = q.x * q.y;
    gl_t tmp2 = q.z * q.w;
    result.m10 = 2.0 * (tmp1 + tmp2) * invs;
    result.m01 = 2.0 * (tmp1 - tmp2) * invs;

    tmp1 = q.x * q.z;
    tmp2 = q.y * q.w;
    result.m20 = 2.0 * (tmp1 - tmp2) * invs;
    result.m02 = 2.0 * (tmp1 + tmp2) * invs;
    tmp1 = q.y * q.z;
    tmp2 = q.x * q.w;
    result.m21 = 2.0 * (tmp1 + tmp2) * invs;
    result.m12 = 2.0 * (tmp1 - tmp2) * invs;

    return result;
}

gl_vec3 gl_vec3_negate(gl_vec3 a) {
    gl_vec3 result;
    for(int32_t i=0; i<3; ++i)
        result.data[i] = -a.data[i];
    return result;
}

gl_vec3 gl_quat_to_euler_angle(gl_vec4 q) {
    gl_t sqw = q.w * q.w;
    gl_t sqx = q.x * q.x;
    gl_t sqy = q.y * q.y;
    gl_t sqz = q.z * q.z;
    gl_t unit = sqx + sqy + sqz + sqw;
    gl_t test = q.x * q.y + q.z * q.w;
    if (test > 0.499 * unit) {
        gl_t y = 2 * gl_atan2(q.x, q.w);
        gl_t z = GL_PI / 2;
        gl_t x = 0;
        return gl_vec3_new(x, y, z);
    }
    if (test < -0.499 * unit) {
        gl_t y = -2 * gl_atan2(q.x, q.w);
        gl_t z = -GL_PI / 2;
        gl_t x = 0;
        return gl_vec3_new(x, y, z);
    }
    gl_t y = gl_atan2(2 * q.y * q.w - 2 * q.x * q.z, sqx - sqy - sqz + sqw);
    gl_t z = gl_asin(2 * test / unit);
    gl_t x = gl_atan2(2 * q.x * q.w - 2 * q.y * q.z, -sqx + sqy - sqz + sqw);
    return gl_vec3_new(x, y, z);
}

gl_vec4 gl_euler_to_angle_quat(gl_vec3 euler) {
    gl_array_op(gl_deg2rad, 3, &euler.data[0]);
    gl_t c1 = gl_cos(euler.y / 2);
    gl_t s1 = gl_sin(euler.y / 2);
    gl_t c2 = gl_cos(euler.z / 2);
    gl_t s2 = gl_sin(euler.z / 2);
    gl_t c3 = gl_cos(euler.x / 2);
    gl_t s3 = gl_sin(euler.x / 2);
    gl_t c1c2 = c1 * c2;
    gl_t s1s2 = s1 * s2;
    gl_t w = c1c2 * c3 - s1s2 * s3;
    gl_t x = c1c2 * s3 + s1s2 * c3;
    gl_t y = s1 * c2 * c3 + c1 * s2 * s3;
    gl_t z = c1 * s2 * c3 - s1 * c2 * s3;
    return gl_vec4_new(x, y, z, w);
}
gl_vec4 gl_quat_conjugate(gl_vec4 q) {
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
    q.w = q.w;
    return q;
}
gl_vec4 gl_quat_normalize(gl_vec4 q){
    gl_t n = gl_sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    q.x /= n;
    q.y /= n;
    q.z /= n;
    q.w /= n;
    return q;
}
gl_vec4 gl_quat_mul(gl_vec4 q1, gl_vec4 q2) {
    gl_t x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
    gl_t y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
    gl_t z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
    gl_t w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
    return gl_vec4_new(x, y, z, w);
}

gl_vec3 gl_quat_mul_vector(gl_vec4 q, gl_vec3 v) {
    //https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
    gl_vec3 u = gl_vec3_new(q.x, q.y, q.z);
    gl_t s = q.w;
    gl_vec3 v1 = gl_vec3_mul(gl_vec3_new_scalar(2.0f * gl_vec3_dot(u, v)), u);
    gl_vec3 v2 = gl_vec3_mul(gl_vec3_new_scalar(s * s - gl_vec3_dot(u, u)), v);
    gl_vec3 v3 = gl_vec3_mul(gl_vec3_new_scalar(2.0f * s), gl_vec3_cross(u, v));
    return gl_vec3_add(v1, gl_vec3_add(v2, v3));
}

void gl_print_vec(float* data, int rows, int cols){
    int count = 0;
    for(int i=0; i<rows; ++i){
        for(int j=0; j<cols; ++j) {
            printf("%f ", *(data + count));
            count++;
        }
        printf("\n");
    }
    printf("\n");
}