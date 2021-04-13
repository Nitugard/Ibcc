/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SOFTFLOAT_H
#define IBC_SOFTFLOAT_H

#include "./Common.h"
#include <stdint.h>


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
    u64 v;
} sf;

API sf sf_new(u64 data);
API sf sf_new_fraction(i64 a, i64 b);
API sf sf_from_raw(i64 a);

API i64 sf_int(sf a);
API f32 sf_float(sf a);
API f64 sf_double(sf a);

/*
 * Conversion functions operating on arrays, convenient for opengl.
 */

API void sf_array_to_int_array(sf const* array, i32 count, i32* out);
API void sf_array_to_float_array(sf const* array, i32 count, f32* out);
API void sf_array_to_double_array(sf const* array, i32 count, f64* out);

/*
 * Functions
 */

API sf sf_abs(sf a);
API sf sf_sqrt(sf a);
API i32 sf_sign(sf a);
API i32 sf_sign_diff(sf a, sf b);
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

API i32 sf_compare(sf a, sf b);
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

API sf sf_array_op_cumm(sf const *a, sf (*op)(sf, sf), i32 count);
API void sf_array_op(sf (*op)(sf), i32 count, sf *array);

API void sf_array_op_variadic(sf(*op)(sf), i32 count, ...);
API sf sf_array_op_cumm_variadic(sf(*op)(sf, sf), i32 count, ...);



#endif //IBC_SOFTFLOAT_H
