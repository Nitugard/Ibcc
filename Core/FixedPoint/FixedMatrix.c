/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "FixedMatrix.h"


#ifndef FP_ASSERT

#include <assert.h>
#include <stdarg.h>

#define FP_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

fp fp_mat_det(fp_mat4 const *a) {

    return FP_ADD(
            FP_MUL(a->m03, a->m12, a->m21, a->m30), FP_MUL_N(a->m02, a->m13, a->m21, a->m30),
            FP_MUL_N(a->m03, a->m11, a->m22, a->m30), FP_MUL(a->m01, a->m13, a->m22, a->m30),
            FP_MUL(a->m02, a->m11, a->m23, a->m30), FP_MUL_N(a->m01, a->m12, a->m23, a->m30),
            FP_MUL_N(a->m03, a->m12, a->m20, a->m31), FP_MUL(a->m02, a->m13, a->m20, a->m31),
            FP_MUL(a->m03, a->m10, a->m22, a->m31), FP_MUL_N(a->m00, a->m13, a->m22, a->m31),
            FP_MUL_N(a->m02, a->m10, a->m23, a->m31), FP_MUL(a->m00, a->m12, a->m23, a->m31),
            FP_MUL(a->m03, a->m11, a->m20, a->m32), FP_MUL_N(a->m01, a->m13, a->m20, a->m32),
            FP_MUL_N(a->m03, a->m10, a->m21, a->m32), FP_MUL(a->m00, a->m13, a->m21, a->m32),
            FP_MUL(a->m01, a->m10, a->m23, a->m32), FP_MUL_N(a->m00, a->m11, a->m23, a->m32),
            FP_MUL_N(a->m02, a->m11, a->m20, a->m33), FP_MUL(a->m01, a->m12, a->m20, a->m33),
            FP_MUL(a->m02, a->m10, a->m21, a->m33), FP_MUL_N(a->m00, a->m12, a->m21, a->m33),
            FP_MUL_N(a->m01, a->m10, a->m22, a->m33), FP_MUL(a->m00, a->m11, a->m22, a->m33));

}

fp_mat4 fp_mat_inverse(fp_mat4 const *a) {
    const fp det = fp_mat_det(a);
    const fp rcp = fp_div(FP_ONE, det);
    FP_ASSERT(fp_not_eq(det, FP_ZERO));

    fp m00 = FP_ADD(FP_MUL(a->m12, a->m23, a->m31), FP_MUL_N(a->m13, a->m22, a->m31), FP_MUL(a->m13, a->m21, a->m32),
                    FP_MUL_N(a->m11, a->m23, a->m32), FP_MUL_N(a->m12, a->m21, a->m33), FP_MUL(a->m11, a->m22, a->m33));
    fp m01 = FP_ADD(FP_MUL(a->m03, a->m22, a->m31), FP_MUL_N(a->m02, a->m23, a->m31), FP_MUL_N(a->m03, a->m21, a->m32),
                    FP_MUL(a->m01, a->m23, a->m32), FP_MUL(a->m02, a->m21, a->m33), FP_MUL_N(a->m01, a->m22, a->m33));
    fp m02 = FP_ADD(FP_MUL(a->m02, a->m13, a->m31), FP_MUL_N(a->m03, a->m12, a->m31), FP_MUL(a->m03, a->m11, a->m32),
                    FP_MUL_N(a->m01, a->m13, a->m32), FP_MUL_N(a->m02, a->m11, a->m33), FP_MUL(a->m01, a->m12, a->m33));
    fp m03 = FP_ADD(FP_MUL(a->m03, a->m12, a->m21), FP_MUL_N(a->m02, a->m13, a->m21), FP_MUL_N(a->m03, a->m11, a->m22),
                    FP_MUL(a->m01, a->m13, a->m22), FP_MUL(a->m02, a->m11, a->m23), FP_MUL_N(a->m01, a->m12, a->m23));
    fp m10 = FP_ADD(FP_MUL(a->m13, a->m22, a->m30), FP_MUL_N(a->m12, a->m23, a->m30), FP_MUL_N(a->m13, a->m20, a->m32),
                    FP_MUL(a->m10, a->m23, a->m32), FP_MUL(a->m12, a->m20, a->m33), FP_MUL_N(a->m10, a->m22, a->m33));
    fp m11 = FP_ADD(FP_MUL(a->m02, a->m23, a->m30), FP_MUL_N(a->m03, a->m22, a->m30), FP_MUL(a->m03, a->m20, a->m32),
                    FP_MUL_N(a->m00, a->m23, a->m32), FP_MUL_N(a->m02, a->m20, a->m33), FP_MUL(a->m00, a->m22, a->m33));
    fp m12 = FP_ADD(FP_MUL(a->m03, a->m12, a->m30), FP_MUL_N(a->m02, a->m13, a->m30), FP_MUL_N(a->m03, a->m10, a->m32),
                    FP_MUL(a->m00, a->m13, a->m32), FP_MUL(a->m02, a->m10, a->m33), FP_MUL_N(a->m00, a->m12, a->m33));
    fp m13 = FP_ADD(FP_MUL(a->m02, a->m13, a->m20), FP_MUL_N(a->m03, a->m12, a->m20), FP_MUL(a->m03, a->m10, a->m22),
                    FP_MUL_N(a->m00, a->m13, a->m22), FP_MUL_N(a->m02, a->m10, a->m23), FP_MUL(a->m00, a->m12, a->m23));
    fp m20 = FP_ADD(FP_MUL(a->m11, a->m23, a->m30), FP_MUL_N(a->m13, a->m21, a->m30), FP_MUL(a->m13, a->m20, a->m31),
                    FP_MUL_N(a->m10, a->m23, a->m31), FP_MUL_N(a->m11, a->m20, a->m33), FP_MUL(a->m10, a->m21, a->m33));
    fp m21 = FP_ADD(FP_MUL(a->m03, a->m21, a->m30), FP_MUL_N(a->m01, a->m23, a->m30), FP_MUL_N(a->m03, a->m20, a->m31),
                    FP_MUL(a->m00, a->m23, a->m31), FP_MUL(a->m01, a->m20, a->m33), FP_MUL_N(a->m00, a->m21, a->m33));
    fp m22 = FP_ADD(FP_MUL(a->m01, a->m13, a->m30), FP_MUL_N(a->m03, a->m11, a->m30), FP_MUL(a->m03, a->m10, a->m31),
                    FP_MUL_N(a->m00, a->m13, a->m31), FP_MUL_N(a->m01, a->m10, a->m33), FP_MUL(a->m00, a->m11, a->m33));
    fp m23 = FP_ADD(FP_MUL(a->m03, a->m11, a->m20), FP_MUL_N(a->m01, a->m13, a->m20), FP_MUL_N(a->m03, a->m10, a->m21),
                    FP_MUL(a->m00, a->m13, a->m21), FP_MUL(a->m01, a->m10, a->m23), FP_MUL_N(a->m00, a->m11, a->m23));
    fp m30 = FP_ADD(FP_MUL(a->m12, a->m21, a->m30), FP_MUL_N(a->m11, a->m22, a->m30), FP_MUL_N(a->m12, a->m20, a->m31),
                    FP_MUL(a->m10, a->m22, a->m31), FP_MUL(a->m11, a->m20, a->m32), FP_MUL_N(a->m10, a->m21, a->m32));
    fp m31 = FP_ADD(FP_MUL(a->m01, a->m22, a->m30), FP_MUL_N(a->m02, a->m21, a->m30), FP_MUL(a->m02, a->m20, a->m31),
                    FP_MUL_N(a->m00, a->m22, a->m31), FP_MUL_N(a->m01, a->m20, a->m32), FP_MUL(a->m00, a->m21, a->m32));
    fp m32 = FP_ADD(FP_MUL(a->m02, a->m11, a->m30), FP_MUL_N(a->m01, a->m12, a->m30), FP_MUL_N(a->m02, a->m10, a->m31),
                    FP_MUL(a->m00, a->m12, a->m31), FP_MUL(a->m01, a->m10, a->m32), FP_MUL_N(a->m00, a->m11, a->m32));
    fp m33 = FP_ADD(FP_MUL(a->m01, a->m12, a->m20), FP_MUL_N(a->m02, a->m11, a->m20), FP_MUL(a->m02, a->m10, a->m21),
                    FP_MUL_N(a->m00, a->m12, a->m21), FP_MUL_N(a->m01, a->m10, a->m22), FP_MUL(a->m00, a->m11, a->m22));


    fp_mat4 res = {
            .data = {
                    m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33
            }
    };

    for (i32 i = 0; i < 16; ++i) res.data[i] = fp_mul(res.data[i], rcp);
    return res;
}

fp_mat4 fp_mat_translate(fp_vec3 const *pos) {
    fp_mat4 result = FP_MAT_IDENTITY;
    result.m03 = pos->x;
    result.m13 = pos->y;
    result.m23 = pos->z;
    return result;
}

fp_mat4 fp_mat_skew(const fp_vec3 *a) {
    fp_mat4 result = {.data =
            {
                    0, fp_negate(a->z), a->y,
                    a->z, 0, fp_negate(a->x),
                    fp_negate(a->y), a->x, 0
            }
    };
    return result;
}

fp_mat4 fp_mat_axis_angle(const fp_vec3 *axis, fp angle) {
    const fp s    = fp_sin(angle);
    const fp c    = fp_cos(angle);
    const fp t    = fp_sub(FP_ONE, c);
    fp_vec3  naxis = *axis;
    fp_vec_normalize(3, naxis.data);
    const fp_mat4 skew  = fp_mat_skew(&naxis);
    const fp_mat4 skew2 = fp_mat_mul(&skew, &skew);
    const fp_mat4 id    = FP_MAT_IDENTITY;
    const fp_mat4 term1 = fp_mat_mul_p(&skew, s);
    const fp_mat4 term2 = fp_mat_mul_p(&skew2, t);
    const fp_mat4 term3 = fp_mat_add(&term1, &term2);
    return fp_mat_add(&id, &term3);
}

fp_mat4 fp_mat_scale(fp_vec3 const *scale) {
    fp_mat4 result = FP_MAT_IDENTITY;
    result.m00 = scale->x;
    result.m11 = scale->y;
    result.m22 = scale->z;
    return result;

}

fp_mat4 fp_mat_mul(fp_mat4 const *a, fp_mat4 const *b) {
    fp_mat4  result;
    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            result.data[i * 4 + j] = fp_vec_dot(fp_mat_row_get(a, i).data, fp_mat_column_get(b, j).data, 4);
        }
    }

    return result;
}

void fp_mat_column_set(fp_vec4 const *a, i32 column, fp_mat4 *b) {
    for (i32 i = 0; i < 4; ++i)
        b->data[column + i * 4] = a->data[i];
}

fp_vec4 fp_mat_column_get(fp_mat4 const *a, i32 column) {
    fp_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = a->data[column + i * 4];
    return result;
}

void fp_mat_row_set(fp_vec4 const *a, i32 row, fp_mat4 *b) {
    for (i32 i = 0; i < 4; ++i)
        b->data[(row * 4) + i] = a->data[i];
}

fp_vec4 fp_mat_row_get(fp_mat4 const *a, i32 row) {
    fp_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = a->data[(row * 4) + i];
    return result;
}

fp_mat4 fp_mat_add(fp_mat4 const *a, fp_mat4 const *b) {
    fp_mat4  result;
    for (i32 i = 0; i < 16; ++i)
        result.data[i] = fp_add(a->data[i], b->data[i]);
    return result;
}

fp_mat4 fp_mat_mul_p(fp_mat4 const *a, fp b) {
    fp_mat4 result;
    for(i32 i=0; i<16; ++i)
        result.data[i] = fp_mul(a->data[i], b);
    return result;
}

fp_mat4 fp_mat_trs(fp_vec3 const *translate, fp_vec3 const *rotate, fp_vec3 const *scale) {
    const fp_mat4 sm = fp_mat_scale(scale);
    const fp_mat4 tm = fp_mat_translate(translate);
    const fp_mat4 rm = fp_mat_rotate_zxy(rotate);
    const fp_mat4 ts = fp_mat_mul(&tm, &sm);
    const fp_mat4 trs = fp_mat_mul(&rm, &ts);
    return trs;
}

fp_mat4 fp_mat_rotate_x(fp angle) {
    const fp c = fp_cos(angle);
    const fp s = fp_sin(angle);
    fp_mat4 result = {.data = {
            FP_ONE,	FP_ZERO,	FP_ZERO, FP_ZERO,
            FP_ZERO,	c, -s, FP_ZERO,
            FP_ZERO,	s,	c, FP_ZERO,
            FP_ZERO,  FP_ZERO,  FP_ZERO,FP_ONE
    }};
    return result;
}

fp_mat4 fp_mat_rotate_y(fp angle) {
    const fp c = fp_cos(angle);
    const fp s = fp_sin(angle);
    fp_mat4 result = {.data ={
            c, FP_ZERO, s, FP_ZERO,
            FP_ZERO, FP_ONE, FP_ZERO, FP_ZERO,
            -s, FP_ZERO, c, FP_ZERO,
            FP_ZERO, FP_ZERO, FP_ZERO,FP_ONE
    }};

    return result;
}

fp_mat4 fp_mat_rotate_z(fp angle) {
    const fp c = fp_cos(angle);
    const fp s = fp_sin(angle);
    fp_mat4 result = {.data ={
            c, -s, FP_ZERO, FP_ZERO,
            s, c, FP_ZERO, FP_ZERO,
            FP_ZERO, FP_ZERO, FP_ONE, FP_ZERO,
            FP_ZERO, FP_ZERO, FP_ZERO, FP_ONE,
    }};
    return result;
}

fp_mat4 fp_mat_rotate_zxy(fp_vec3 const *euler_angles) {
    const fp_mat4 rz = fp_mat_rotate_z(euler_angles->z);
    const fp_mat4 rx = fp_mat_rotate_x(euler_angles->x);
    const fp_mat4 ry = fp_mat_rotate_y(euler_angles->y);
    const fp_mat4 zx = fp_mat_mul(&rx, &rz);
    const fp_mat4 zxy = fp_mat_mul(&ry, &zx);
    return zxy;
}

fp_mat4 fp_mat_perspective(fp fov, fp ar, fp near, fp far) {
    fp cot = FP_DIV(FP_ONE, fp_tan(FP_MUL(fov, FP_DIV(FP_PI, fp_from_int(360)))));
    struct fp_mat4 result = FP_MAT_ZERO;
    result.m00 = FP_DIV(cot, ar);
    result.m11 = cot;
    result.m32 = FP_NEG_ONE;
    result.m22 = FP_DIV(FP_ADD(near, far), FP_SUB(near, far));
    result.m23 = FP_DIV(FP_MUL(fp_from_int(2), near, far), FP_SUB(near, far));
    result.m33 = FP_ZERO;
    return result;
}

fp_mat4 fp_mat_transpose(fp_mat4 const *a) {
    struct fp_mat4 result;
    for(i32 c = 0; c < 4; ++c) {
        for (i32 r = 0; r < 4; ++r) {
            result.data[c + r * 4] = a->data[r + c * 4];
        }
    }

    return result;
}

fp_mat4 fp_mat_op_cumm(fp_mat4 const *a, fp_mat4 (*op)(fp_mat4 const*, fp_mat4 const*), i32 count) {
    fp_mat4 res = a[0];
    for (i32 i = 1; i < count; ++i) res = op(&res, &a[i]);
    return res;
}

fp_mat4 fp_mat_op_cumm_variadic(fp_mat4(*op)(fp_mat4 const*, fp_mat4 const*), i32 count, ...) {
    va_list args;
    va_start(args, count);
    FP_ASSERT(count > 1);
    fp_mat4* num = va_arg(args, fp_mat4*);
    for (i32 i = 1; i < count; ++i) {
        *num = op(num, va_arg(args, fp_mat4*));
    }
    va_end(args);
    return *num;
}
