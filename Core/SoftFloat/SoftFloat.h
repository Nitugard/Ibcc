/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef SOFTFLOAT_H
#define SOFTFLOAT_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif


#ifndef PP_NARG
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0            \

#endif
#define SF_ADD(...) sf_array_op_cumm_variadic(sf_add, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define SF_SUB(...) sf_array_op_cumm_variadic(sf_sub, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define SF_MUL(...) sf_array_op_cumm_variadic(sf_mul, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define SF_DIV(...) sf_array_op_cumm_variadic(sf_div, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define SF_NEG(V) sf_negate(V)
#define SF_MUL_N(...) SF_NEG(SF_MUL(__VA_ARGS__))
#define SF_OP2_VA(OP2, ...) sf_array_op_cumm_variadic(OP2, NARGS(__VA_ARGS__), __VA_ARGS__)


//IEEE754
#define SF_PI sf_from_raw(0x400921FB54442D18ULL)
#define SF_PI2 sf_from_raw(0x401921FB54442D18ULL)
#define SF_E sf_from_raw(0x4005BF0A8B145769ULL)

#define SF_ONE sf_from_raw(0x3FF0000000000000ULL)
#define SF_180 sf_from_raw(0x4066800000000000ULL)
#define SF_HALF sf_from_raw(0x3FE0000000000000ULL)
#define SF_THIRD sf_from_raw(0x3FD5555555555555ULL)
#define SF_RCP_180 sf_from_raw(0x3F76C16C16C16C17ULL)
#define SF_RCP_PI sf_from_raw(0x3FD45F306DC9C883ULL)
#define SF_ZERO sf_from_raw(0x0ULL)

typedef struct sf{
    uint64_t v;
} sf;

API sf sf_new(uint64_t data);
API sf sf_new_fraction(int64_t a, int64_t b);
API sf sf_from_raw(int64_t a);

API int64_t sf_int(sf a);
API float sf_float(sf a);
API double sf_double(sf a);

/*
 * Conversion functions operating on arrays, convenient for opengl.
 */

API void sf_array_to_int_array(sf const* array, int32_t count, int32_t* out);
API void sf_array_to_float_array(sf const* array, int32_t count, float* out);
API void sf_array_to_double_array(sf const* array, int32_t count, double* out);

/*
 * Functions
 */

API sf sf_abs(sf a);
API sf sf_sqrt(sf a);
API int32_t sf_sign(sf a);
API int32_t sf_sign_diff(sf a, sf b);
API sf sf_negate(sf a);
API sf sf_min(sf a, sf b);
API sf sf_max(sf a, sf b);
API sf sf_deg2rad(sf a);
API sf sf_rad2deg(sf a);
API sf sf_round_to_int(sf a);
API sf sf_floor(sf a);
API sf sf_fract(sf a);
API sf sf_ceil(sf a);

/*
 * Operators
 */

API sf sf_add(sf a, sf b);
API sf sf_sub(sf a, sf b);
API sf sf_mul(sf a, sf b);
API sf sf_div(sf a, sf b);

/*
 * Logic operators
 */

API int32_t sf_compare(sf a, sf b);
API bool sf_less_equal(sf a, sf b);
API bool sf_less(sf a, sf b);
API bool sf_equal(sf a, sf b);

/*
 * Trig functions
 */

API sf sf_sin(sf a);
API sf sf_cos(sf a);
API sf sf_tan(sf a);

API sf sf_asin(sf a);
API sf sf_acos(sf a);
API sf sf_atan(sf a);
API sf sf_atan2(sf a, sf b);

/*
 * Array operators
 */

API sf sf_array_op_cumm(sf const *a, sf (*op)(sf, sf), int32_t count);
API void sf_array_op(sf (*op)(sf), int32_t count, sf *array);

API void sf_array_op_variadic(sf(*op)(sf), int32_t count, ...);
API sf sf_array_op_cumm_variadic(sf(*op)(sf, sf), int32_t count, ...);



#endif //SOFTFLOAT_H

#ifdef SOFTFLOAT_IMPLEMENTATION
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
#undef SOFTFLOAT_IMPLEMENTATION
#endif