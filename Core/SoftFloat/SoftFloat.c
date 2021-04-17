/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "SoftFloat.h"
#include <softfloat.h>
#include <math.h>
#include <stdarg.h>

#ifndef CORE_ASSERT
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif


#define core_t(x) *((double*)&((x).v))
#define base_t(x) *((float64_t*)&(x))
#define super_t(x) *((sf*)&(x))


sf sf_new(uint64_t a) {
    float64_t res = i64_to_f64(a);
    return super_t(res);
}

sf sf_new_fraction(int64_t a, int64_t b) {
    CORE_ASSERT(b != 0);
    return SF_DIV(sf_new(a), sf_new(b));
}

sf sf_from_raw(int64_t a) {
    sf res;
    res.v = a;
    return res;
}

int64_t sf_int(sf a) {
    return f64_to_i64(base_t(a), softfloat_round_max, 0);
}


float sf_float(sf a) {
    return (float)sf_double(a);
}

double sf_double(sf a) {
    return core_t(a);
}

sf sf_abs(sf a) {
    a.v &= (1ULL << 63) - 1;
    return a;
}

sf sf_sqrt(sf a) {
    float64_t res = f64_sqrt(base_t(a));
    return super_t(res);
}

int32_t sf_sign(sf a) {
    int32_t sign = (a.v & (1ULL << 63)) >> 63;
    if(sign > 0) return -1;
    return 1;
}

int32_t sf_sign_diff(sf a, sf b) {
    return sf_sign(a) * sf_sign(b);
}

sf sf_negate(sf a) {
    a.v = (a.v ^ (1ULL << 63));
    return a;
}

sf sf_min(sf a, sf b) {
    if(sf_compare(a, b) > 0)
        return b;
    return a;
}

sf sf_max(sf a, sf b) {
    if(sf_compare(a, b) > 0)
        return a;
    return b;
}

int32_t sf_compare(sf a, sf b) {
    if(f64_eq(base_t(a), base_t(b)))
        return 0;
    if(f64_lt(base_t(a), base_t(b)))
        return -1;
    return 1;
}


sf sf_deg2rad(sf a) {
    return SF_MUL(a, SF_PI, SF_RCP_180);
}

sf sf_rad2deg(sf a) {
    return SF_MUL(a, SF_RCP_PI, SF_180);
}

sf sf_round_to_int(sf a) {
    float64_t res = f64_roundToInt(base_t(a), softfloat_round_near_even, 0);
    return super_t(res);
}

sf sf_floor(sf a) {
    float64_t res = f64_roundToInt(base_t(a), softfloat_round_min, 0);
    return super_t(res);
}

sf sf_ceil(sf a) {
    float64_t res = f64_roundToInt(base_t(a), softfloat_round_max, 0);
    return super_t(res);

}

sf sf_add(sf a, sf b) {
    float64_t res = f64_add(base_t(a), base_t(b));
    return super_t(res);
}

sf sf_sub(sf a, sf b) {
    float64_t res = f64_sub(base_t(a), base_t(b));
    return super_t(res);
}

sf sf_mul(sf a, sf b) {
    float64_t res = f64_mul(base_t(a), base_t(b));
    return super_t(res);
}

sf sf_div(sf a, sf b) {
    float64_t res = f64_div(base_t(a), base_t(b));
    return super_t(res);
}

bool sf_less_equal(sf a, sf b) {
    return f64_le(base_t(a), base_t(b));
}

bool sf_less(sf a, sf b) {
    return f64_lt(base_t(a), base_t(b));
}

bool sf_equal(sf a, sf b) {
    return f64_eq(base_t(a), base_t(b));

}

sf sf_cos(sf a) {
    double res = cos(core_t(a));
    return super_t(res);
}

sf sf_sin(sf a) {
    double res = sin(core_t(a));
    return super_t(res);

}

sf sf_tan(sf a) {
    double res = tan(core_t(a));
    return super_t(res);
}

sf sf_asin(sf a) {
    double res = asin(core_t(a));
    return super_t(res);

}

sf sf_acos(sf a) {
    double res = acos(core_t(a));
    return super_t(res);
}

sf sf_atan(sf a) {
    double res = atan(core_t(a));
    return super_t(res);
}

sf sf_atan2(sf a, sf b) {
    double res = atan2(core_t(a), core_t(b));
    return super_t(res);
}


sf sf_array_op_cumm(sf const *a, sf (*op)(sf, sf), int32_t count) {
    sf res = a[0];
    for (int32_t i = 1; i < count; ++i) res = op(res, a[i]);
    return res;
}

void sf_array_op(sf (*op)(sf), int32_t count, sf *array) {
    for (int32_t i = 0; i < count; ++i) array[i] = op(array[i]);
}

sf sf_array_op_cumm_variadic(sf(*op)(sf, sf), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    sf num = va_arg(args, sf);
    for (int32_t i = 1; i < count; ++i)
        num = op(num, va_arg(args, sf));
    va_end(args);
    return num;
}

void sf_array_op_variadic(sf (*op)(sf), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    for (int32_t i = 1; i < count; ++i) {
        sf *num = va_arg(args, sf*);
        *num = op(*num);
    }
    va_end(args);
}

void sf_array_to_int_array(const sf *array, int32_t count, int32_t *out) {
    for(int32_t i=0; i < count; ++i) out[i] = sf_int(array[i]);
}

void sf_array_to_float_array(const sf *array, int32_t count, float *out) {
    for(int32_t i=0; i < count; ++i) out[i] = sf_float(array[i]);
}

void sf_array_to_double_array(const sf *array, int32_t count, double *out) {
    for(int32_t i=0; i < count; ++i) out[i] = sf_double(array[i]);
}