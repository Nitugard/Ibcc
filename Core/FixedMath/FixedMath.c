/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "FixedMath.h"
#include "FixedMathLut.h"

#include <math.h>
#include <stdarg.h>

#define FP_FRACTION_MASK (((1ULL << 32)-1))
#define FP_INTEGER_MASK (~((1ULL << 32)-1))

i32 uint64_leading_zeros(u64 x);

void divllu(u64 u0, u64 u1, u64 v, u64 *q, u64 *r);

fp fp_from_int(i32 value) {
    return (i64) value << 32;
}

fp fp_from_uninit_fp(fp value) {
    return fp_from_int(value);
}

fp fp_from_double(f64 value) {
    return (i64) (value * (1ULL << 32));
}

i32 fp_to_int(fp a) {
    return a >> 32;
}

f64 fp_to_double(fp a) {
    return (a * (1.0 / (1ULL << 32)));
}

fp fp_abs_p(fp a) {
    if (fp_less_p(a, FP_ZERO)) return fp_negate_p(a);
    else return a;
}

i32 fp_signdiff_p(fp a, fp b) {
    return fp_sign_p(a) * fp_sign_p(b);
}

fp fp_add_p(fp a, fp b) {
    #ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) + fp_to_double(b));
    #else
    #ifdef DEBUG
    i64 abs_a, abs_b;
    abs_a     = fp_abs_p(a);
    abs_b     = fp_abs_p(b);
    i64 sum = (abs_a +abs_b);
    FP_ASSERT(sum >= abs_a);
    #endif

    return a + b;
    #endif
}

fp fp_sub_p(fp a, fp b) {
    #ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) - fp_to_double(b));
    #else
    #ifdef DEBUG
    i64 abs_a, abs_b;
    abs_a     = fp_abs_p(a);
    abs_b     = fp_abs_p(b);
    i64 subtract = (abs_a - abs_b);
    FP_ASSERT(subtract <= abs_a);
    #endif

    return a - b;
    #endif
}

fp fp_mul_p(fp a, fp b) {
    #ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) * fp_to_double(b));
    #else
    fp  abs_a, abs_b;
    u64 hi_a, hi_b, lo_a, lo_b, hi_hi;
    i32 sign_diff;
    abs_a     = fp_abs_p(a);
    abs_b     = fp_abs_p(b);
    sign_diff = fp_signdiff_p(a, b);
    hi_a      = abs_a >> 32;
    hi_b      = abs_b >> 32;
    lo_a      = abs_a & 0xFFFFFFFF;
    lo_b      = abs_b & 0xFFFFFFFF;
    hi_hi     = (hi_a * hi_b);
    FP_ASSERT(hi_hi < 0x40000000);
    return sign_diff * ((hi_hi << 32) +
           (hi_a * lo_b) + (hi_b * lo_a) +
           ((lo_a * lo_b) >> 32));
    #endif
}

fp fp_div_p(fp a, fp b) {
    FP_ASSERT(fp_not_eq_p(a, FP_ZERO));
    #ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) / fp_to_double(b));
    #else
    u64 abs_a, abs_b;
    u64 quo;
    i32 sign_diff;
    abs_a     = fp_abs_p(a);
    abs_b     = fp_abs_p(b);
    sign_diff = fp_signdiff_p(a, b);
    divllu(abs_a << 32, abs_a >> 32, abs_b, &quo, 0);
    FP_ASSERT((i64)quo >= 0);
    return (i64)quo * sign_diff;
    #endif
}

fp fp_sqrt_p(fp a) {
    #ifdef FLOAT_MATH
    return fp_from_double(sqrt(fp_to_double(a)));
    #else
    // Adapted from https://github.com/chmike/fpsqrt
    FP_ASSERT(fp_greater_p(a, FP_ZERO));

    u64 r = (u64)a;
    u64 b = (u64)(0x4000000000000000);
    u64 q = 0ULL;

    while (b > 0x40)
    {
        u64 t = q + b;
        if (r >= t)
        {
            r -= t;
            q = t + b;
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 16;
    return (i64)q;
    #endif
}

i32 fp_sign_p(fp a) {
    if (fp_greater_p(a, FP_ZERO)) return 1;
    else return -1;
}

fp fp_unit_sin_p(fp x) {
    //x / pi * table_size * 2
    fp  lutIndexFp = fp_mul_p(fp_mul_p(x, FP_PI_RCP), fp_from_int(FpSinLutSegments * 2));
    i32 lutIndex   = fp_to_int(lutIndexFp);
    FP_ASSERT(lutIndex < FpSinLutSegments);
    lutIndex *= 3;
    fp a = FpSinLut[lutIndex + 2];
    fp b = FpSinLut[lutIndex + 1];
    fp c = FpSinLut[lutIndex];
    //a + b*x + c*x^2
    return fp_add_p(a, fp_add_p(fp_mul_p(x, b), fp_mul_p(fp_mul_p(x, x), c)));
}

fp fp_sin_p(fp x) {

    #ifdef FLOAT_MATH
    return fp_from_double(sin(fp_to_double(x)));
    #else

    fp circle_x = fp_mod_p(x, FP_PI2);
    if (fp_less_p(circle_x, FP_ZERO))
        circle_x = fp_add_p(circle_x, FP_PI2);

    if (fp_less_or_eq_p(circle_x, FP_PI_HALF))
        return fp_unit_sin_p(circle_x);

    if (fp_less_or_eq_p(circle_x, FP_PI))
        return fp_unit_sin_p(fp_sub_p(FP_PI, circle_x));

    if (fp_less_or_eq_p(circle_x, FP_PI3O2))
        return fp_negate_p(fp_unit_sin_p(fp_sub_p(circle_x, FP_PI)));

    return fp_negate_p(fp_unit_sin_p(fp_sub_p(FP_PI2, circle_x)));
    #endif
}

fp fp_cos_p(fp x) {
    #ifdef FLOAT_MATH
    return fp_from_double(cos(fp_to_double(x)));
    #else

    fp circle_x = fp_mod_p(x, FP_PI2);
    if (fp_less_p(circle_x, FP_ZERO))
        circle_x = fp_add_p(circle_x, FP_PI2);

    if (fp_less_or_eq_p(circle_x, FP_PI_HALF))
        return fp_unit_sin_p(fp_sub_p(FP_PI_HALF, circle_x));

    if (fp_less_or_eq_p(circle_x, FP_PI))
        return fp_negate_p(fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(FP_PI, circle_x))));

    if (fp_less_or_eq_p(circle_x, FP_PI3O2))
        return fp_negate_p(fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(circle_x, FP_PI))));

    return fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(FP_PI2, circle_x)));
    #endif
}

fp fp_tan_p(fp x) {
    #ifdef FLOAT_MATH
    return fp_from_double(tan(fp_to_double(x)));
    #else
    fp circle_x = fp_mod_p(x, FP_PI2);
    if (fp_less_p(circle_x, FP_ZERO))
        circle_x = fp_add_p(circle_x, FP_PI2);

    if (fp_less_or_eq_p(circle_x, FP_PI_HALF))
        return fp_div_p(fp_unit_sin_p(circle_x),
                         fp_unit_sin_p(fp_sub_p(FP_PI_HALF, circle_x)));

    if (fp_less_or_eq_p(circle_x, FP_PI))
        return fp_negate_p(fp_div_p(fp_unit_sin_p(fp_sub_p(FP_PI, circle_x)),
                          fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(FP_PI, circle_x)))));

    if (fp_less_or_eq_p(circle_x, FP_PI3O2))
        return fp_div_p(fp_unit_sin_p(fp_sub_p(circle_x, FP_PI)),
                         fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(circle_x, FP_PI))));

    return fp_negate_p(fp_div_p(fp_unit_sin_p(fp_sub_p(FP_PI2, circle_x)),
                      fp_unit_sin_p(fp_sub_p(FP_PI_HALF, fp_sub_p(FP_PI2, circle_x)))));
    #endif
}

fp fp_mod_p(fp a, fp b) {
    FP_ASSERT(fp_greater_p(b, FP_ZERO));
    fp nb = fp_negate_p(b);
    while (fp_greater_or_eq_p(a, b))
        a = fp_sub_p(a, b);
    while (fp_less_or_eq_p(a, nb))
        a = fp_add_p(a, b);
    return a;
}

fp fp_negate_p(fp a) {
    return -a;
}

//TODO: ADD FIXED IMPL

fp fp_asin_p(fp a) {
    #ifdef FLOAT_MATH
    #else
    #endif
    return fp_from_double(asin(fp_to_double(a)));
}

fp fp_acos_p(fp a) {
    #ifdef FLOAT_MATH
    #else
    #endif
    return fp_from_double(acos(fp_to_double(a)));
}

fp fp_atan_p(fp a) {
    #ifdef FLOAT_MATH
    #else
    #endif
    return fp_from_double(atan(fp_to_double(a)));
}

fp fp_atan2_p(fp a, fp b) {
    #ifdef FLOAT_MATH
    #else
    #endif
    return fp_from_double(atan2(fp_to_double(a), fp_to_double(b)));
}

void fp_add_v(fp const *a, fp const *b, i32 len, fp *res) {
    for (i32 i = 0; i < len; ++i) res[i] = fp_add_p(a[i], b[i]);
}

void fp_sub_v(fp const *a, fp const *b, i32 len, fp *res) {
    for (i32 i = 0; i < len; ++i) res[i] = fp_sub_p(a[i], b[i]);
}

void fp_mul_v(fp const *a, fp const *b, i32 len, fp *res) {
    for (i32 i = 0; i < len; ++i) res[i] = fp_mul_p(a[i], b[i]);
}

void fp_div_v(fp const *a, fp const *b, i32 len, fp *res) {
    for (i32 i = 0; i < len; ++i) res[i] = fp_div_p(a[i], b[i]);
}


fp fp_op2_v(const fp *a, fp (*op)(fp, fp), i32 len) {
    fp       res = a[0];
    for (i32 i   = 1; i < len; ++i) res = op(res, a[i]);
    return res;
}

void fp_op_v(fp (*op)(fp), i32 len, fp *a) {
    for (i32 i = 0; i < len; ++i) a[i] = op(a[i]);
}

fp fp_max_p(fp a, fp b) {
    if (a > b)
        return a;
    return b;
}

fp fp_min_p(fp a, fp b) {
    if (a > b)
        return b;
    return a;
}

fp fp_round_p(fp a) {
    #ifdef FLOAT_MATH
    return fp_from_int(round(fp_to_double(a)));
    #else
    return fp_from_int(fp_to_int(fp_add_p(a, FP_HALF)));
    #endif
}

fp fp_dot_v(const fp *a, const fp *b, i32 len) {
    fp       res = FP_ZERO;
    for (i32 i   = 0; i < len; ++i) res = fp_add_p(res, fp_mul_p(a[i], b[i]));
    return res;
}

fp fp_norm_sqr_v(const fp *a, i32 len) {
    return fp_dot_v(a, a, len);
}

fp fp_norm_v(const fp *a, i32 len) {
    return fp_sqrt_p(fp_norm_sqr_v(a, len));
}

void fp_normalize_v(i32 len, fp *a) {
    fp       norm = fp_norm_v(a, len);
    fp       rcp  = fp_div_p(FP_ONE, norm);
    for (i32 i    = 0; i < len; ++i) a[i] = fp_mul_p(a[i], rcp);
}

fp_vec3 fp_cross_v(const fp_vec3 *a, const fp_vec3 *b) {
    fp_vec3 res = {
            .x = fp_sub_p(fp_mul_p(a->y, b->z), fp_mul_p(a->z, b->y)),
            .y = fp_sub_p(fp_mul_p(a->z, b->x), fp_mul_p(a->x, b->z)),
            .z = fp_sub_p(fp_mul_p(a->x, b->y), fp_mul_p(a->y, b->x))
    };
    return res;
}

fp fp_angle_v(const fp_vec3 *a, const fp_vec3 *b) {
    return fp_acos_p(fp_div_p(fp_dot_v(a->data, b->data, 3),
                              fp_mul_p(fp_norm_v(a->data, 3), fp_norm_v(b->data, 3))));
}

fp fp_det_m(const fp_mat4 *m1) {

    return FP_ADD(
            FP_MUL(m1->m03, m1->m12, m1->m21, m1->m30), FP_MUL_N(m1->m02, m1->m13, m1->m21, m1->m30),
            FP_MUL_N(m1->m03, m1->m11, m1->m22, m1->m30), FP_MUL(m1->m01, m1->m13, m1->m22, m1->m30),
            FP_MUL(m1->m02, m1->m11, m1->m23, m1->m30), FP_MUL_N(m1->m01, m1->m12, m1->m23, m1->m30),
            FP_MUL_N(m1->m03, m1->m12, m1->m20, m1->m31), FP_MUL(m1->m02, m1->m13, m1->m20, m1->m31),
            FP_MUL(m1->m03, m1->m10, m1->m22, m1->m31), FP_MUL_N(m1->m00, m1->m13, m1->m22, m1->m31),
            FP_MUL_N(m1->m02, m1->m10, m1->m23, m1->m31), FP_MUL(m1->m00, m1->m12, m1->m23, m1->m31),
            FP_MUL(m1->m03, m1->m11, m1->m20, m1->m32), FP_MUL_N(m1->m01, m1->m13, m1->m20, m1->m32),
            FP_MUL_N(m1->m03, m1->m10, m1->m21, m1->m32), FP_MUL(m1->m00, m1->m13, m1->m21, m1->m32),
            FP_MUL(m1->m01, m1->m10, m1->m23, m1->m32), FP_MUL_N(m1->m00, m1->m11, m1->m23, m1->m32),
            FP_MUL_N(m1->m02, m1->m11, m1->m20, m1->m33), FP_MUL(m1->m01, m1->m12, m1->m20, m1->m33),
            FP_MUL(m1->m02, m1->m10, m1->m21, m1->m33), FP_MUL_N(m1->m00, m1->m12, m1->m21, m1->m33),
            FP_MUL_N(m1->m01, m1->m10, m1->m22, m1->m33), FP_MUL(m1->m00, m1->m11, m1->m22, m1->m33));

}

fp_mat4 fp_inv_m(const fp_mat4 *m) {
    const fp det = fp_det_m(m);
    const fp rcp = fp_div_p(FP_ONE, det);
    FP_ASSERT(fp_not_eq_p(det, FP_ZERO));

    fp m00 = FP_ADD(FP_MUL(m->m12, m->m23, m->m31), FP_MUL_N(m->m13, m->m22, m->m31), FP_MUL(m->m13, m->m21, m->m32),
                    FP_MUL_N(m->m11, m->m23, m->m32), FP_MUL_N(m->m12, m->m21, m->m33), FP_MUL(m->m11, m->m22, m->m33));
    fp m01 = FP_ADD(FP_MUL(m->m03, m->m22, m->m31), FP_MUL_N(m->m02, m->m23, m->m31), FP_MUL_N(m->m03, m->m21, m->m32),
                    FP_MUL(m->m01, m->m23, m->m32), FP_MUL(m->m02, m->m21, m->m33), FP_MUL_N(m->m01, m->m22, m->m33));
    fp m02 = FP_ADD(FP_MUL(m->m02, m->m13, m->m31), FP_MUL_N(m->m03, m->m12, m->m31), FP_MUL(m->m03, m->m11, m->m32),
                    FP_MUL_N(m->m01, m->m13, m->m32), FP_MUL_N(m->m02, m->m11, m->m33), FP_MUL(m->m01, m->m12, m->m33));
    fp m03 = FP_ADD(FP_MUL(m->m03, m->m12, m->m21), FP_MUL_N(m->m02, m->m13, m->m21), FP_MUL_N(m->m03, m->m11, m->m22),
                    FP_MUL(m->m01, m->m13, m->m22), FP_MUL(m->m02, m->m11, m->m23), FP_MUL_N(m->m01, m->m12, m->m23));
    fp m10 = FP_ADD(FP_MUL(m->m13, m->m22, m->m30), FP_MUL_N(m->m12, m->m23, m->m30), FP_MUL_N(m->m13, m->m20, m->m32),
                    FP_MUL(m->m10, m->m23, m->m32), FP_MUL(m->m12, m->m20, m->m33), FP_MUL_N(m->m10, m->m22, m->m33));
    fp m11 = FP_ADD(FP_MUL(m->m02, m->m23, m->m30), FP_MUL_N(m->m03, m->m22, m->m30), FP_MUL(m->m03, m->m20, m->m32),
                    FP_MUL_N(m->m00, m->m23, m->m32), FP_MUL_N(m->m02, m->m20, m->m33), FP_MUL(m->m00, m->m22, m->m33));
    fp m12 = FP_ADD(FP_MUL(m->m03, m->m12, m->m30), FP_MUL_N(m->m02, m->m13, m->m30), FP_MUL_N(m->m03, m->m10, m->m32),
                    FP_MUL(m->m00, m->m13, m->m32), FP_MUL(m->m02, m->m10, m->m33), FP_MUL_N(m->m00, m->m12, m->m33));
    fp m13 = FP_ADD(FP_MUL(m->m02, m->m13, m->m20), FP_MUL_N(m->m03, m->m12, m->m20), FP_MUL(m->m03, m->m10, m->m22),
                    FP_MUL_N(m->m00, m->m13, m->m22), FP_MUL_N(m->m02, m->m10, m->m23), FP_MUL(m->m00, m->m12, m->m23));
    fp m20 = FP_ADD(FP_MUL(m->m11, m->m23, m->m30), FP_MUL_N(m->m13, m->m21, m->m30), FP_MUL(m->m13, m->m20, m->m31),
                    FP_MUL_N(m->m10, m->m23, m->m31), FP_MUL_N(m->m11, m->m20, m->m33), FP_MUL(m->m10, m->m21, m->m33));
    fp m21 = FP_ADD(FP_MUL(m->m03, m->m21, m->m30), FP_MUL_N(m->m01, m->m23, m->m30), FP_MUL_N(m->m03, m->m20, m->m31),
                    FP_MUL(m->m00, m->m23, m->m31), FP_MUL(m->m01, m->m20, m->m33), FP_MUL_N(m->m00, m->m21, m->m33));
    fp m22 = FP_ADD(FP_MUL(m->m01, m->m13, m->m30), FP_MUL_N(m->m03, m->m11, m->m30), FP_MUL(m->m03, m->m10, m->m31),
                    FP_MUL_N(m->m00, m->m13, m->m31), FP_MUL_N(m->m01, m->m10, m->m33), FP_MUL(m->m00, m->m11, m->m33));
    fp m23 = FP_ADD(FP_MUL(m->m03, m->m11, m->m20), FP_MUL_N(m->m01, m->m13, m->m20), FP_MUL_N(m->m03, m->m10, m->m21),
                    FP_MUL(m->m00, m->m13, m->m21), FP_MUL(m->m01, m->m10, m->m23), FP_MUL_N(m->m00, m->m11, m->m23));
    fp m30 = FP_ADD(FP_MUL(m->m12, m->m21, m->m30), FP_MUL_N(m->m11, m->m22, m->m30), FP_MUL_N(m->m12, m->m20, m->m31),
                    FP_MUL(m->m10, m->m22, m->m31), FP_MUL(m->m11, m->m20, m->m32), FP_MUL_N(m->m10, m->m21, m->m32));
    fp m31 = FP_ADD(FP_MUL(m->m01, m->m22, m->m30), FP_MUL_N(m->m02, m->m21, m->m30), FP_MUL(m->m02, m->m20, m->m31),
                    FP_MUL_N(m->m00, m->m22, m->m31), FP_MUL_N(m->m01, m->m20, m->m32), FP_MUL(m->m00, m->m21, m->m32));
    fp m32 = FP_ADD(FP_MUL(m->m02, m->m11, m->m30), FP_MUL_N(m->m01, m->m12, m->m30), FP_MUL_N(m->m02, m->m10, m->m31),
                    FP_MUL(m->m00, m->m12, m->m31), FP_MUL(m->m01, m->m10, m->m32), FP_MUL_N(m->m00, m->m11, m->m32));
    fp m33 = FP_ADD(FP_MUL(m->m01, m->m12, m->m20), FP_MUL_N(m->m02, m->m11, m->m20), FP_MUL(m->m02, m->m10, m->m21),
                    FP_MUL_N(m->m00, m->m12, m->m21), FP_MUL_N(m->m01, m->m10, m->m22), FP_MUL(m->m00, m->m11, m->m22));


    fp_mat4 res = {
            .data = {
                    m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33
            }
    };

    for (i32 i = 0; i < 16; ++i) res.data[i] = fp_mul_p(res.data[i], rcp);
    return res;
}

fp_mat4 fp_translate_m(const fp_vec3 *a) {
    fp_mat4 result = FP_MAT_IDENTITY;
    result.m03 = a->x;
    result.m13 = a->y;
    result.m23 = a->z;
    return result;
}

fp_mat4 fp_skew_m(const fp_vec3 *v) {
    fp_mat4 result = {.data =
            {
                    0, fp_negate_p(v->z), v->y,
                    v->z, 0, fp_negate_p(v->x),
                    fp_negate_p(v->y), v->x, 0
            }
    };
    return result;
}

fp_mat4 fp_axis_angle_m(const fp_vec3 *a, fp angle) {
    const fp s    = fp_sin_p(angle);
    const fp c    = fp_cos_p(angle);
    const fp t    = fp_sub_p(FP_ONE, c);
    fp_vec3  axis = *a;
    fp_normalize_v(3, axis.data);
    const fp_mat4 skew  = fp_skew_m(&axis);
    const fp_mat4 skew2 = fp_mul_m(&skew, &skew);
    const fp_mat4 id    = FP_MAT_IDENTITY;
    const fp_mat4 term1 = fp_mul_mp(&skew, s);
    const fp_mat4 term2 = fp_mul_mp(&skew2, t);
    const fp_mat4 term3 = fp_add_m(&term1, &term2);
    return fp_add_m(&id, &term3);
}

fp_mat4 fp_scale_m(const fp_vec3 *a) {
    fp_mat4 result = FP_MAT_IDENTITY;
    result.m00 = a->x;
    result.m11 = a->y;
    result.m22 = a->z;
    return result;

}

fp_mat4 fp_mul_m(const fp_mat4 *a, const fp_mat4 *b) {
    fp_mat4  result;
    for (i32 i = 0; i < 16; ++i)
        result.data[i] = fp_dot_v(fp_row_m_get(a, i).data, fp_col_m_get(b, i).data, 4);
    return result;
}

void fp_col_m_set(const fp_vec4 *v, i32 c, fp_mat4 *m) {
    for (i32 i = 0; i < 4; ++i)
        m->data[c + i * 4] = v->data[i];
}

fp_vec4 fp_col_m_get(const fp_mat4 *m, i32 c) {
    fp_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = m->data[c + i * 4];
    return result;
}

void fp_row_m_set(const fp_vec4 *v, i32 r, fp_mat4 *m) {
    for (i32 i = 0; i < 4; ++i)
        m->data[(r * 4) + (i % 4)] = v->data[i];
}

fp_vec4 fp_row_m_get(const fp_mat4 *m, i32 r) {
    fp_vec4  result;
    for (i32 i = 0; i < 4; ++i)
        result.data[i] = m->data[(r * 4) + (i % 4)];
    return result;
}

fp_mat4 fp_add_m(const fp_mat4 * a, const fp_mat4 * b) {
    fp_mat4  result;
    for (i32 i = 0; i < 16; ++i)
        result.data[i] = fp_add_p(a->data[i], b->data[i]);
    return result;
}

fp_mat4 fp_mul_mp(const fp_mat4 * a, fp b) {
    fp_mat4 result;
    for(i32 i=0; i<16; ++i)
        result.data[i] = fp_mul_p(a->data[i], b);
    return result;
}

fp_mat4 fp_trs_m(const fp_vec3 * t, const fp_vec3 *r, const fp_vec3 *s) {
    const fp_mat4 scale = fp_scale_m(s);
    const fp_mat4 translate = fp_translate_m(t);
    const fp_mat4 rot = fp_rotateZXY_m(r);
    const fp_mat4 ts = fp_mul_m(&translate, &scale);
    const fp_mat4 trs = fp_mul_m(&rot, &ts);
    return trs;
}

fp_mat4 fp_rotateX_m(fp angle) {
    const fp c = cos(angle);
    const fp s = sin(angle);
    fp_mat4 result = {.data = {
            1,	0,	0,
            0,	c, -s,
            0,	s,	c,
    }};
    return result;
}

fp_mat4 fp_rotateY_m(fp angle) {
    const fp c = cos(angle);
    const fp s = sin(angle);
    fp_mat4 result = {.data ={
             c, 0, s,
             0, 1, 0,
            -s, 0, c,
    }};

    return result;
}

fp_mat4 fp_rotateZ_m(fp angle) {
    const fp c = cos(angle);
    const fp s = sin(angle);
    fp_mat4 result = {.data ={
            c, -s,	0,
            s,	c,	0,
            0,	0,	1,
    }};
    return result;
}

fp_mat4 fp_rotateZXY_m(const fp_vec3 * m) {
    const fp_mat4 rz = fp_rotateZ_m(m->z);
    const fp_mat4 rx = fp_rotateX_m(m->x);
    const fp_mat4 ry = fp_rotateY_m(m->y);
    const fp_mat4 zx = fp_mul_m(&rx, &rz);
    const fp_mat4 zxy = fp_mul_m(&ry, &zx);
    return zxy;
}

fp fp_deg2rad_p(fp a) {
    return fp_mul_p(a, FP_DEG2RAD);
}

fp fp_rad2deg_p(fp a) {
    return fp_mul_p(a, FP_RAD2DEG);
}

fp fp_floor_p(fp a) {
    return (a & FP_INTEGER_MASK);
}

fp fp_ceil_p(fp a) {
    return ((a + FP_FRACTION_MASK) & FP_INTEGER_MASK);
}

fp fp_fract_p(fp a) {
    return a & FP_FRACTION_MASK;
}

fp fp_op2_p_va(fp(*op)(fp, fp), i32 count, ...) {
    va_list args;
    va_start(args, count);
    FP_ASSERT(count > 1);
    fp       num = va_arg(args, fp);
    for (i32 i   = 1; i < count; ++i)
        num = op(num, va_arg(args, fp));
    va_end(args);
    return num;
}

void fp_op_p_va(fp (*op)(fp), i32 count, ...) {
    va_list args;
    va_start(args, count);
    FP_ASSERT(count > 1);
    for (i32 i   = 1; i < count; ++i) {
        fp* num = va_arg(args, fp*);
        *num = op(*num);
    }
    va_end(args);
}

i32 fp_compare_p(fp a, fp b) {
    if(a > b) return 1;
    if(a == b) return 0;
    return -1;
}

i32 fp_less_p(fp a, fp b) {
    return a < b;
}

i32 fp_greater_p(fp a, fp b) {
    return a > b;
}

i32 fp_less_or_eq_p(fp a, fp b) {
    return a <= b;
}

i32 fp_greater_or_eq_p(fp a, fp b) {
    return a >= b;
}

i32 fp_equal_p(fp a, fp b) {
    return a == b;
}

i32 fp_not_eq_p(fp a, fp b) {
    return a != b;
}

void divllu(u64 u0, u64 u1, u64 v, u64 *q, u64 *r) {

    //https://skanthak.homepage.t-online.de/division.html
    u64 qhat;        // A quotient.
    u64 rhat;        // A remainder.
    u64 uhat;        // A dividend digit pair.
    u32 q0, q1;      // Quotient digits.
    u32 s;           // Shift amount for norm.

    FP_ASSERT(u1 < v);                  // If overflow

    s    = __builtin_clzll(v);         // 0 <= s <= 63.
    if (s != 0) {
        v <<= s;                    // Normalize divisor.
        u1 <<= s;                   // Shift dividend left.
        u1 |= u0 >> (64 - s);
        u0 <<= s;
    }
    // Compute high quotient digit.
    qhat = u1 / (u32) (v >> 32);
    rhat = u1 - (u32) (v >> 32) * qhat; //    rhat = u1 % (unsigned) (v >> 32);


    while ((u32) (qhat >> 32) != 0U ||
           // Both qhat and rhat are less 2**32 here!
           (u64) (u32) (qhat & ~0U) * (u32) (v & ~0U) >
           ((rhat << 32) | (u32) (u0 >> 32))) {
        qhat -= 1;
        rhat += (u32) (v >> 32);
        if ((u32) (rhat >> 32) != 0U) break;
    }

    q1   = (u32) (qhat & ~0U);
    // Multiply and subtract.
    uhat = ((u1 << 32) | (u32) (u0 >> 32)) - q1 * v;

    // Compute low quotient digit.
    qhat = uhat / (u32) (v >> 32);
    rhat = uhat - (qhat * (u32) (v >> 32)); //    rhat = uhat % (unsigned) (v >> 32);


    while ((u32) (qhat >> 32) != 0U ||
           // Both qhat and rhat are less 2**32 here!
           (u64) (u32) (qhat & ~0U) * (u32) (v & ~0U) >
           ((rhat << 32) | (u32) (u0 & ~0U))) {
        qhat -= 1;
        rhat += (u32) (v >> 32);
        if ((u32) (rhat >> 32) != 0U) break;
    }

    q0 = (u32) (qhat & ~0U);

    if (r != 0)                  // If remainder is wanted, return it.
        *r = (((uhat << 32) | (u32) (u0 & ~0U)) - q0 * v) >> s;

    *q = ((u64) q1 << 32) | q0;
}

i32 uint64_leading_zeros(u64 x) {
    int n = 0;
    if (x == 0) { return 64; }
    if (x <= 0x00000000FFFFFFFFL) {
        n = n + 32;
        x = x << 32;
    }
    if (x <= 0x0000FFFFFFFFFFFFL) {
        n = n + 16;
        x = x << 16;
    }
    if (x <= 0x00FFFFFFFFFFFFFFL) {
        n = n + 8;
        x = x << 8;
    }
    if (x <= 0x0FFFFFFFFFFFFFFFL) {
        n = n + 4;
        x = x << 4;
    }
    if (x <= 0x3FFFFFFFFFFFFFFFL) {
        n = n + 2;
        x = x << 2;
    }
    if (x <= 0x7FFFFFFFFFFFFFFFL) { n = n + 1; }
    return n;
}
