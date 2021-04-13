/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "SoftMatrix.h"


#ifndef SF_ASSERT

#include <assert.h>
#include <stdarg.h>
#define SF_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

sf sf_mat_det(sf_mat4 const *a) {

    return SF_ADD(
            SF_MUL(a->m03, a->m12, a->m21, a->m30), SF_MUL_N(a->m02, a->m13, a->m21, a->m30),
            SF_MUL_N(a->m03, a->m11, a->m22, a->m30), SF_MUL(a->m01, a->m13, a->m22, a->m30),
            SF_MUL(a->m02, a->m11, a->m23, a->m30), SF_MUL_N(a->m01, a->m12, a->m23, a->m30),
            SF_MUL_N(a->m03, a->m12, a->m20, a->m31), SF_MUL(a->m02, a->m13, a->m20, a->m31),
            SF_MUL(a->m03, a->m10, a->m22, a->m31), SF_MUL_N(a->m00, a->m13, a->m22, a->m31),
            SF_MUL_N(a->m02, a->m10, a->m23, a->m31), SF_MUL(a->m00, a->m12, a->m23, a->m31),
            SF_MUL(a->m03, a->m11, a->m20, a->m32), SF_MUL_N(a->m01, a->m13, a->m20, a->m32),
            SF_MUL_N(a->m03, a->m10, a->m21, a->m32), SF_MUL(a->m00, a->m13, a->m21, a->m32),
            SF_MUL(a->m01, a->m10, a->m23, a->m32), SF_MUL_N(a->m00, a->m11, a->m23, a->m32),
            SF_MUL_N(a->m02, a->m11, a->m20, a->m33), SF_MUL(a->m01, a->m12, a->m20, a->m33),
            SF_MUL(a->m02, a->m10, a->m21, a->m33), SF_MUL_N(a->m00, a->m12, a->m21, a->m33),
            SF_MUL_N(a->m01, a->m10, a->m22, a->m33), SF_MUL(a->m00, a->m11, a->m22, a->m33));

}

sf_mat4 sf_mat_inverse(sf_mat4 const *a) {
    const sf det = sf_mat_det(a);
    const sf rcp = SF_DIV(SF_ONE, det);
    SF_ASSERT(!sf_equal(det, SF_ZERO));

    sf m00 = SF_ADD(SF_MUL(a->m12, a->m23, a->m31), SF_MUL_N(a->m13, a->m22, a->m31), SF_MUL(a->m13, a->m21, a->m32),
                    SF_MUL_N(a->m11, a->m23, a->m32), SF_MUL_N(a->m12, a->m21, a->m33), SF_MUL(a->m11, a->m22, a->m33));
    sf m01 = SF_ADD(SF_MUL(a->m03, a->m22, a->m31), SF_MUL_N(a->m02, a->m23, a->m31), SF_MUL_N(a->m03, a->m21, a->m32),
                    SF_MUL(a->m01, a->m23, a->m32), SF_MUL(a->m02, a->m21, a->m33), SF_MUL_N(a->m01, a->m22, a->m33));
    sf m02 = SF_ADD(SF_MUL(a->m02, a->m13, a->m31), SF_MUL_N(a->m03, a->m12, a->m31), SF_MUL(a->m03, a->m11, a->m32),
                    SF_MUL_N(a->m01, a->m13, a->m32), SF_MUL_N(a->m02, a->m11, a->m33), SF_MUL(a->m01, a->m12, a->m33));
    sf m03 = SF_ADD(SF_MUL(a->m03, a->m12, a->m21), SF_MUL_N(a->m02, a->m13, a->m21), SF_MUL_N(a->m03, a->m11, a->m22),
                    SF_MUL(a->m01, a->m13, a->m22), SF_MUL(a->m02, a->m11, a->m23), SF_MUL_N(a->m01, a->m12, a->m23));
    sf m10 = SF_ADD(SF_MUL(a->m13, a->m22, a->m30), SF_MUL_N(a->m12, a->m23, a->m30), SF_MUL_N(a->m13, a->m20, a->m32),
                    SF_MUL(a->m10, a->m23, a->m32), SF_MUL(a->m12, a->m20, a->m33), SF_MUL_N(a->m10, a->m22, a->m33));
    sf m11 = SF_ADD(SF_MUL(a->m02, a->m23, a->m30), SF_MUL_N(a->m03, a->m22, a->m30), SF_MUL(a->m03, a->m20, a->m32),
                    SF_MUL_N(a->m00, a->m23, a->m32), SF_MUL_N(a->m02, a->m20, a->m33), SF_MUL(a->m00, a->m22, a->m33));
    sf m12 = SF_ADD(SF_MUL(a->m03, a->m12, a->m30), SF_MUL_N(a->m02, a->m13, a->m30), SF_MUL_N(a->m03, a->m10, a->m32),
                    SF_MUL(a->m00, a->m13, a->m32), SF_MUL(a->m02, a->m10, a->m33), SF_MUL_N(a->m00, a->m12, a->m33));
    sf m13 = SF_ADD(SF_MUL(a->m02, a->m13, a->m20), SF_MUL_N(a->m03, a->m12, a->m20), SF_MUL(a->m03, a->m10, a->m22),
                    SF_MUL_N(a->m00, a->m13, a->m22), SF_MUL_N(a->m02, a->m10, a->m23), SF_MUL(a->m00, a->m12, a->m23));
    sf m20 = SF_ADD(SF_MUL(a->m11, a->m23, a->m30), SF_MUL_N(a->m13, a->m21, a->m30), SF_MUL(a->m13, a->m20, a->m31),
                    SF_MUL_N(a->m10, a->m23, a->m31), SF_MUL_N(a->m11, a->m20, a->m33), SF_MUL(a->m10, a->m21, a->m33));
    sf m21 = SF_ADD(SF_MUL(a->m03, a->m21, a->m30), SF_MUL_N(a->m01, a->m23, a->m30), SF_MUL_N(a->m03, a->m20, a->m31),
                    SF_MUL(a->m00, a->m23, a->m31), SF_MUL(a->m01, a->m20, a->m33), SF_MUL_N(a->m00, a->m21, a->m33));
    sf m22 = SF_ADD(SF_MUL(a->m01, a->m13, a->m30), SF_MUL_N(a->m03, a->m11, a->m30), SF_MUL(a->m03, a->m10, a->m31),
                    SF_MUL_N(a->m00, a->m13, a->m31), SF_MUL_N(a->m01, a->m10, a->m33), SF_MUL(a->m00, a->m11, a->m33));
    sf m23 = SF_ADD(SF_MUL(a->m03, a->m11, a->m20), SF_MUL_N(a->m01, a->m13, a->m20), SF_MUL_N(a->m03, a->m10, a->m21),
                    SF_MUL(a->m00, a->m13, a->m21), SF_MUL(a->m01, a->m10, a->m23), SF_MUL_N(a->m00, a->m11, a->m23));
    sf m30 = SF_ADD(SF_MUL(a->m12, a->m21, a->m30), SF_MUL_N(a->m11, a->m22, a->m30), SF_MUL_N(a->m12, a->m20, a->m31),
                    SF_MUL(a->m10, a->m22, a->m31), SF_MUL(a->m11, a->m20, a->m32), SF_MUL_N(a->m10, a->m21, a->m32));
    sf m31 = SF_ADD(SF_MUL(a->m01, a->m22, a->m30), SF_MUL_N(a->m02, a->m21, a->m30), SF_MUL(a->m02, a->m20, a->m31),
                    SF_MUL_N(a->m00, a->m22, a->m31), SF_MUL_N(a->m01, a->m20, a->m32), SF_MUL(a->m00, a->m21, a->m32));
    sf m32 = SF_ADD(SF_MUL(a->m02, a->m11, a->m30), SF_MUL_N(a->m01, a->m12, a->m30), SF_MUL_N(a->m02, a->m10, a->m31),
                    SF_MUL(a->m00, a->m12, a->m31), SF_MUL(a->m01, a->m10, a->m32), SF_MUL_N(a->m00, a->m11, a->m32));
    sf m33 = SF_ADD(SF_MUL(a->m01, a->m12, a->m20), SF_MUL_N(a->m02, a->m11, a->m20), SF_MUL(a->m02, a->m10, a->m21),
                    SF_MUL_N(a->m00, a->m12, a->m21), SF_MUL_N(a->m01, a->m10, a->m22), SF_MUL(a->m00, a->m11, a->m22));


    sf_mat4 res = {
            .data = {
                    m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33
            }
    };

    for (i32 i = 0; i < 16; ++i) res.data[i] = SF_MUL(res.data[i], rcp);
    return res;
}

sf_mat4 sf_mat_translate(sf_vec3 const *pos) {
    sf_mat4 result = SF_MAT_IDENTITY;
    result.m03 = pos->x;
    result.m13 = pos->y;
    result.m23 = pos->z;
    return result;
}

sf_mat4 sf_mat_skew(const sf_vec3 *a) {
    sf_mat4 result = {.data =
            {
                    SF_ZERO, sf_negate(a->z), a->y,
                    a->z, SF_ZERO, sf_negate(a->x),
                    sf_negate(a->y), a->x, SF_ZERO
            }
    };
    return result;
}

sf_mat4 sf_mat_axis_angle(const sf_vec3 *axis, sf angle) {
    const sf s    = sf_sin(angle);
    const sf c    = sf_cos(angle);
    const sf t    = SF_SUB(SF_ONE, c);
    sf_vec3  naxis = *axis;
    sf_vec_normalize(3, naxis.data);
    const sf_mat4 skew  = sf_mat_skew(&naxis);
    const sf_mat4 skew2 = sf_mat_mul(&skew, &skew);
    const sf_mat4 id    = SF_MAT_IDENTITY;
    const sf_mat4 term1 = sf_mat_mul_p(&skew, s);
    const sf_mat4 term2 = sf_mat_mul_p(&skew2, t);
    const sf_mat4 term3 = sf_mat_add(&term1, &term2);
    return sf_mat_add(&id, &term3);
}

sf_mat4 sf_mat_scale(sf_vec3 const *scale) {
    sf_mat4 result = SF_MAT_IDENTITY;
    result.m00 = scale->x;
    result.m11 = scale->y;
    result.m22 = scale->z;
    return result;

}

sf_mat4 sf_mat_mul(sf_mat4 const *a, sf_mat4 const *b) {
    sf_mat4  result;
    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            result.data[i * 4 + j] = sf_vec_dot(sf_mat_row_get(a, i).data, sf_mat_column_get(b, j).data, 4);
        }
    }

    return result;
}

void sf_mat_column_set(sf_vec4 const *a, i32 column, sf_mat4 *b) {
    for (i32 i = 0; i < 4; ++i)
        b->data[column + i * 4] = a->data[i];
}

sf_vec4 sf_mat_column_get(sf_mat4 const *a, i32 column) {
    sf_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = a->data[column + i * 4];
    return result;
}

void sf_mat_row_set(sf_vec4 const *a, i32 row, sf_mat4 *b) {
    for (i32 i = 0; i < 4; ++i)
        b->data[(row * 4) + i] = a->data[i];
}

sf_vec4 sf_mat_row_get(sf_mat4 const *a, i32 row) {
    sf_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = a->data[(row * 4) + i];
    return result;
}

sf_mat4 sf_mat_add(sf_mat4 const *a, sf_mat4 const *b) {
    sf_mat4  result;
    for (i32 i = 0; i < 16; ++i)
        result.data[i] = SF_ADD(a->data[i], b->data[i]);
    return result;
}

sf_mat4 sf_mat_mul_p(sf_mat4 const *a, sf b) {
    sf_mat4 result;
    for(i32 i=0; i<16; ++i)
        result.data[i] = SF_MUL(a->data[i], b);
    return result;
}

sf_mat4 sf_mat_trs(sf_vec3 const *translate, sf_vec3 const *rotate, sf_vec3 const *scale) {
    const sf_mat4 sm = sf_mat_scale(scale);
    const sf_mat4 tm = sf_mat_translate(translate);
    const sf_mat4 rm = sf_mat_rotate_zxy(rotate);
    const sf_mat4 ts = sf_mat_mul(&tm, &sm);
    const sf_mat4 trs = sf_mat_mul(&rm, &ts);
    return trs;
}

sf_mat4 sf_mat_rotate_x(sf angle) {
    const sf c = sf_cos(angle);
    const sf s = sf_sin(angle);
    sf_mat4 result = {.data = {
            SF_ONE,	SF_ZERO,	SF_ZERO, SF_ZERO,
            SF_ZERO,	c, sf_negate(s), SF_ZERO,
            SF_ZERO,	s,	c, SF_ZERO,
            SF_ZERO,  SF_ZERO,  SF_ZERO,SF_ONE
    }};
    return result;
}

sf_mat4 sf_mat_rotate_y(sf angle) {
    const sf c = sf_cos(angle);
    const sf s = sf_sin(angle);
    sf_mat4 result = {.data ={
            c, SF_ZERO, s, SF_ZERO,
            SF_ZERO, SF_ONE, SF_ZERO, SF_ZERO,
            sf_negate(s), SF_ZERO, c, SF_ZERO,
            SF_ZERO, SF_ZERO, SF_ZERO,SF_ONE
    }};

    return result;
}

sf_mat4 sf_mat_rotate_z(sf angle) {
    const sf c = sf_cos(angle);
    const sf s = sf_sin(angle);
    sf_mat4 result = {.data ={
            c, sf_negate(s), SF_ZERO, SF_ZERO,
            s, c, SF_ZERO, SF_ZERO,
            SF_ZERO, SF_ZERO, SF_ONE, SF_ZERO,
            SF_ZERO, SF_ZERO, SF_ZERO, SF_ONE,
    }};
    return result;
}

sf_mat4 sf_mat_rotate_zxy(sf_vec3 const *euler_angles) {
    const sf_mat4 rz = sf_mat_rotate_z(euler_angles->z);
    const sf_mat4 rx = sf_mat_rotate_x(euler_angles->x);
    const sf_mat4 ry = sf_mat_rotate_y(euler_angles->y);
    const sf_mat4 zx = sf_mat_mul(&rx, &rz);
    const sf_mat4 zxy = sf_mat_mul(&ry, &zx);
    return zxy;
}

sf_mat4 sf_mat_perspective(sf fov, sf ar, sf near, sf far) {
    sf cot = SF_DIV(SF_ONE, sf_tan(SF_MUL(fov, SF_DIV(SF_PI, sf_new(360)))));
    struct sf_mat4 result = SF_MAT_ZERO;
    result.m00 = SF_DIV(cot, ar);
    result.m11 = cot;
    result.m32 = sf_negate(SF_ONE);
    result.m22 = SF_DIV(SF_ADD(near, far), SF_SUB(near, far));
    result.m23 = SF_DIV(SF_MUL(sf_new(2), near, far), SF_SUB(near, far));
    result.m33 = SF_ZERO;
    return result;
}

sf_mat4 sf_mat_transpose(sf_mat4 const *a) {
    struct sf_mat4 result;
    for(i32 c = 0; c < 4; ++c) {
        for (i32 r = 0; r < 4; ++r) {
            result.data[c + r * 4] = a->data[r + c * 4];
        }
    }

    return result;
}

sf_mat4 sf_mat_op_cumm(sf_mat4 const *a, sf_mat4 (*op)(sf_mat4 const*, sf_mat4 const*), i32 count) {
    sf_mat4 res = a[0];
    for (i32 i = 1; i < count; ++i) res = op(&res, &a[i]);
    return res;
}

sf_mat4 sf_mat_op_cumm_variadic(sf_mat4(*op)(sf_mat4 const*, sf_mat4 const*), i32 count, ...) {
    va_list args;
    va_start(args, count);
    SF_ASSERT(count > 1);
    sf_mat4* num = va_arg(args, sf_mat4*);
    for (i32 i = 1; i < count; ++i) {
        *num = op(num, va_arg(args, sf_mat4*));
    }
    va_end(args);
    return *num;
}
