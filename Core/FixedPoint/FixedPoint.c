/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "FixedPoint.h"
#include "FixedPointLut.h"

#include <math.h>
#include <stdarg.h>


#ifndef FP_ASSERT

#include <assert.h>

#define FP_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

#define FP_FRACTION_MASK (((1ULL << 32)-1))
#define FP_INTEGER_MASK (~((1ULL << 32)-1))

i32 uint64_leading_zeros(u64 x);

void divllu(u64 u0, u64 u1, u64 v, u64 *q, u64 *r);

fp fp_from_int(i32 a) {
    return (i64) a << 32;
}

fp fp_from_double(f64 a) {
    return (i64) (a * (1ULL << 32));
}

i32 fp_to_int(fp a) {
    return a >> 32;
}

f64 fp_to_double(fp a) {
    return (a * (1.0 / (1ULL << 32)));
}

fp fp_abs(fp a) {
    if (fp_less(a, FP_ZERO)) return fp_negate(a);
    else return a;
}

i32 fp_sign_diff(fp a, fp b) {
    return fp_sign(a) * fp_sign(b);
}

fp fp_sqrt(fp a) {
#ifdef FLOAT_MATH
    return fp_from_double(sqrt(fp_to_double(a)));
#else
    // Adapted from https://github.com/chmike/fpsqrt
    FP_ASSERT(fp_greater(a, FP_ZERO));

    u64 r = (u64) a;
    u64 b = (u64) (0x4000000000000000);
    u64 q = 0ULL;

    while (b > 0x40) {
        u64 t = q + b;
        if (r >= t) {
            r -= t;
            q = t + b;
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 16;
    return (i64) q;
#endif
}

i32 fp_sign(fp a) {
    if (fp_greater(a, FP_ZERO)) return 1;
    else return -1;
}

fp fp_unit_sin_p(fp x) {
    //x / pi * table_size * 2
    fp lutIndexFp = fp_mul(fp_mul(x, FP_PI_RCP), fp_from_int(FpSinLutSegments * 2));
    i32 lutIndex = fp_to_int(lutIndexFp);
    FP_ASSERT(lutIndex < FpSinLutSegments);
    lutIndex *= 3;
    fp a = FpSinLut[lutIndex + 2];
    fp b = FpSinLut[lutIndex + 1];
    fp c = FpSinLut[lutIndex];
    //a + b*x + c*x^2
    return fp_add(a, fp_add(fp_mul(x, b), fp_mul(fp_mul(x, x), c)));
}

fp fp_max(fp a, fp b) {
    if (a > b)
        return a;
    return b;
}

fp fp_min(fp a, fp b) {
    if (a > b)
        return b;
    return a;
}

fp fp_round(fp a) {
#ifdef FLOAT_MATH
    return fp_from_int(round(fp_to_double(a)));
#else
    return fp_from_int(fp_to_int(fp_add(a, FP_HALF)));
#endif
}

fp fp_deg2rad(fp a) {
    return fp_mul(a, FP_DEG2RAD);
}

fp fp_rad2deg(fp a) {
    return fp_mul(a, FP_RAD2DEG);
}

fp fp_floor(fp a) {
    return (a & FP_INTEGER_MASK);
}

fp fp_ceil(fp a) {
    return ((a + FP_FRACTION_MASK) & FP_INTEGER_MASK);
}

fp fp_fract(fp a) {
    return a & FP_FRACTION_MASK;
}

fp fp_mod(fp a, fp b) {
    FP_ASSERT(fp_greater(b, FP_ZERO));
    fp nb = fp_negate(b);
    while (fp_greater_or_eq(a, b))
        a = fp_sub(a, b);
    while (fp_less_or_eq(a, nb))
        a = fp_add(a, b);
    return a;
}

fp fp_negate(fp a) {
    return -a;
}


fp fp_add(fp a, fp b) {
#ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) + fp_to_double(b));
#else
    return a + b;
#endif
}

fp fp_sub(fp a, fp b) {
#ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) - fp_to_double(b));
#else
    return a - b;
#endif
}

fp fp_mul(fp a, fp b) {
#ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) * fp_to_double(b));
#else
    fp abs_a, abs_b;
    u64 hi_a, hi_b, lo_a, lo_b, hi_hi;
    i32 sign_diff;
    abs_a = fp_abs(a);
    abs_b = fp_abs(b);
    sign_diff = fp_sign_diff(a, b);
    hi_a = abs_a >> 32;
    hi_b = abs_b >> 32;
    lo_a = abs_a & 0xFFFFFFFF;
    lo_b = abs_b & 0xFFFFFFFF;
    hi_hi = (hi_a * hi_b);
    return sign_diff * ((hi_hi << 32) +
                        (hi_a * lo_b) + (hi_b * lo_a) +
                        ((lo_a * lo_b) >> 32));
#endif
}

fp fp_div(fp a, fp b) {
    FP_ASSERT(fp_not_eq(a, FP_ZERO));
#ifdef FLOAT_MATH
    return fp_from_double(fp_to_double(a) / fp_to_double(b));
#else
    u64 abs_a, abs_b;
    u64 quo;
    i32 sign_diff;
    abs_a = fp_abs(a);
    abs_b = fp_abs(b);
    sign_diff = fp_sign_diff(a, b);
    divllu(abs_a << 32, abs_a >> 32, abs_b, &quo, 0);
    FP_ASSERT((i64) quo >= 0);
    return (i64) quo * sign_diff;
#endif
}


i32 fp_compare(fp a, fp b) {
    if (a > b) return 1;
    if (a == b) return 0;
    return -1;
}

i32 fp_less(fp a, fp b) {
    return a < b;
}

i32 fp_greater(fp a, fp b) {
    return a > b;
}

i32 fp_less_or_eq(fp a, fp b) {
    return a <= b;
}

i32 fp_greater_or_eq(fp a, fp b) {
    return a >= b;
}

i32 fp_equal(fp a, fp b) {
    return a == b;
}

i32 fp_not_eq(fp a, fp b) {
    return a != b;
}


fp fp_sin(fp a) {

#ifdef FLOAT_MATH
    return fp_from_double(sin(fp_to_double(a)));
#else

    fp circle_x = fp_mod(a, FP_PI2);
    if (fp_less(circle_x, FP_ZERO))
        circle_x = fp_add(circle_x, FP_PI2);

    if (fp_less_or_eq(circle_x, FP_PI_HALF))
        return fp_unit_sin_p(circle_x);

    if (fp_less_or_eq(circle_x, FP_PI))
        return fp_unit_sin_p(fp_sub(FP_PI, circle_x));

    if (fp_less_or_eq(circle_x, FP_PI3O2))
        return fp_negate(fp_unit_sin_p(fp_sub(circle_x, FP_PI)));

    return fp_negate(fp_unit_sin_p(fp_sub(FP_PI2, circle_x)));
#endif
}

fp fp_cos(fp a) {
#ifdef FLOAT_MATH
    return fp_from_double(cos(fp_to_double(a)));
#else

    fp circle_x = fp_mod(a, FP_PI2);
    if (fp_less(circle_x, FP_ZERO))
        circle_x = fp_add(circle_x, FP_PI2);

    if (fp_less_or_eq(circle_x, FP_PI_HALF))
        return fp_unit_sin_p(fp_sub(FP_PI_HALF, circle_x));

    if (fp_less_or_eq(circle_x, FP_PI))
        return fp_negate(fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(FP_PI, circle_x))));

    if (fp_less_or_eq(circle_x, FP_PI3O2))
        return fp_negate(fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(circle_x, FP_PI))));

    return fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(FP_PI2, circle_x)));
#endif
}

fp fp_tan(fp a) {
#ifdef FLOAT_MATH
    return fp_from_double(tan(fp_to_double(a)));
#else
    fp circle_x = fp_mod(a, FP_PI2);
    if (fp_less(circle_x, FP_ZERO))
        circle_x = fp_add(circle_x, FP_PI2);

    if (fp_less_or_eq(circle_x, FP_PI_HALF))
        return fp_div(fp_unit_sin_p(circle_x),
                      fp_unit_sin_p(fp_sub(FP_PI_HALF, circle_x)));

    if (fp_less_or_eq(circle_x, FP_PI))
        return fp_negate(fp_div(fp_unit_sin_p(fp_sub(FP_PI, circle_x)),
                                fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(FP_PI, circle_x)))));

    if (fp_less_or_eq(circle_x, FP_PI3O2))
        return fp_div(fp_unit_sin_p(fp_sub(circle_x, FP_PI)),
                      fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(circle_x, FP_PI))));

    return fp_negate(fp_div(fp_unit_sin_p(fp_sub(FP_PI2, circle_x)),
                            fp_unit_sin_p(fp_sub(FP_PI_HALF, fp_sub(FP_PI2, circle_x)))));
#endif
}

fp fp_asin(fp a) {
#ifdef FLOAT_MATH
#else
#endif
    return fp_from_double(asin(fp_to_double(a)));
}

fp fp_acos(fp a) {
#ifdef FLOAT_MATH
#else
#endif
    return fp_from_double(acos(fp_to_double(a)));
}

fp fp_atan(fp a) {
#ifdef FLOAT_MATH
#else
#endif
    return fp_from_double(atan(fp_to_double(a)));
}

fp fp_atan2(fp a, fp b) {
#ifdef FLOAT_MATH
#else
#endif
    return fp_from_double(atan2(fp_to_double(a), fp_to_double(b)));
}


fp fp_array_op_cumm(fp const *a, fp (*op)(fp, fp), i32 count) {
    fp res = a[0];
    for (i32 i = 1; i < count; ++i) res = op(res, a[i]);
    return res;
}

void fp_array_op(fp (*op)(fp), i32 count, fp *array) {
    for (i32 i = 0; i < count; ++i) array[i] = op(array[i]);
}

fp fp_array_op_cumm_variadic(fp(*op)(fp, fp), i32 count, ...) {
    va_list args;
    va_start(args, count);
    FP_ASSERT(count > 1);
    fp num = va_arg(args, fp);
    for (i32 i = 1; i < count; ++i)
        num = op(num, va_arg(args, fp));
    va_end(args);
    return num;
}

void fp_array_op_variadic(fp (*op)(fp), i32 count, ...) {
    va_list args;
    va_start(args, count);
    FP_ASSERT(count > 1);
    for (i32 i = 1; i < count; ++i) {
        fp *num = va_arg(args, fp*);
        *num = op(*num);
    }
    va_end(args);
}

void fp_array_to_int_array(const fp *array, i32 count, f32 *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_to_int(array[i]);
}

void fp_array_to_float_array(const fp *array, i32 count, f32 *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_to_double(array[i]);
}

void fp_array_to_double_array(const fp *array, i32 count, f64 *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_to_double(array[i]);
}

void fp_array_from_int_array(const i32 *array, i32 count, fp *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_from_int(array[i]);
}

void fp_array_from_float_array(const f32 *array, i32 count, fp *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_from_double(array[i]);
}

void fp_array_from_double_array(const f64 *array, i32 count, fp *out) {
    for(i32 i=0; i<count; ++i) out[i] = fp_from_double(array[i]);
}


void divllu(u64 u0, u64 u1, u64 v, u64 *q, u64 *r) {

    //https://skanthak.homepage.t-online.de/division.html
    u64 qhat;        // A quotient.
    u64 rhat;        // A remainder.
    u64 uhat;        // A dividend digit pair.
    u32 q0, q1;      // Quotient digits.
    u32 s;           // Shift amount for norm.

    FP_ASSERT(u1 < v);                  // If overflow

    s = __builtin_clzll(v);         // 0 <= s <= 63.
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

    q1 = (u32) (qhat & ~0U);
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
