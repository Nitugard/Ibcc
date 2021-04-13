/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

 //Todo: Compare

#ifndef FIXEDPHYSICS_FIXEDMATH_H
#define FIXEDPHYSICS_FIXEDMATH_H

#include "../Common.h"

typedef i64 fp;
typedef struct fp_vec3 fp_vec3;
typedef struct fp_vec4 fp_vec4;
typedef struct fp_mat4 fp_mat4;

#define DEBUG
//#define FLOAT_MATH

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
#define FP_ADD(...) fp_array_op_cumm_variadic(fp_add, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_SUB(...) fp_array_op_cumm_variadic(fp_sub, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_MUL(...) fp_array_op_cumm_variadic(fp_mul, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_DIV(...) fp_array_op_cumm_variadic(fp_div, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_NEG(V) fp_negate(V)
#define FP_MUL_N(...) FP_NEG(FP_MUL(__VA_ARGS__))

#define FP_OP2_VA(OP2, ...) fp_array_op_cumm_variadic(OP2, NARGS(__VA_ARGS__), __VA_ARGS__)

#define FP_ZERO (0)
#define FP_ONE (1ULL<<32)
#define FP_NEG_ONE (fp_negate(1LL << 32))
#define FP_HALF (1ULL<<31)
#define FP_THIRD (1431655765)
#define FP_QUARTER (1ULL<<30)

#define FP_PI2 (26986075409LL)
#define FP_PI (13493037705LL)
#define FP_PI_HALF (13493037705LL >> 1)
#define FP_PI_RCP (1367130551LL)
#define FP_PI2_RCP (683565275LL)
#define FP_180_RCP (23860929LL)
#define FP_PI3O2 (20239556556LL)
#define FP_DEG2RAD (74961321LL)
#define FP_RAD2DEG (246083499208LL)


API fp fp_from_int(i32 a);
API fp fp_from_double(f64 a);

API i32 fp_to_int(fp a);
API f64 fp_to_double(fp a);

API void fp_array_to_int_array(fp const* array, i32 count, f32* out);
API void fp_array_to_float_array(fp const* array, i32 count, f32* out);
API void fp_array_to_double_array(fp const* array, i32 count, f64* out);

API void fp_array_from_int_array(const i32 *array, i32 count, fp *out);
API void fp_array_from_float_array(const f32* array, i32 count, fp *out);
API void fp_array_from_double_array(const f64* array, i32 count, fp *out);

/*
 * Functions
 */

API fp fp_abs(fp a);
API fp fp_sqrt(fp a);
API i32 fp_sign(fp a);
API i32 fp_sign_diff(fp a, fp b);
API fp fp_mod(fp a, fp b);
API fp fp_negate(fp a);
API fp fp_min(fp a, fp b);
API fp fp_max(fp a, fp b);
API fp fp_deg2rad(fp a);
API fp fp_rad2deg(fp a);
API fp fp_round(fp a);
API fp fp_floor(fp a);
API fp fp_fract(fp a);
API fp fp_ceil(fp a);

/*
 * Operators
 */

API fp fp_add(fp a, fp b);
API fp fp_sub(fp a, fp b);
API fp fp_mul(fp a, fp b);
API fp fp_div(fp a, fp b);

/*
 * Logic operators
 */

API i32 fp_compare(fp a, fp b);
API i32 fp_less(fp a, fp b);
API i32 fp_greater(fp a, fp b);
API i32 fp_equal(fp a, fp b);
API i32 fp_not_eq(fp a, fp b);
API i32 fp_less_or_eq(fp a, fp b);
API i32 fp_greater_or_eq(fp a, fp b);

/*
 * Trig functions
 */

API fp fp_sin(fp a);
API fp fp_cos(fp a);
API fp fp_tan(fp a);

API fp fp_asin(fp a);
API fp fp_acos(fp a);
API fp fp_atan(fp a);
API fp fp_atan2(fp a, fp b);

/*
 * Array operators
 */

API fp fp_array_op_cumm(fp const *a, fp (*op)(fp, fp), i32 count);
API void fp_array_op(fp (*op)(fp), i32 count, fp *array);

API void fp_array_op_variadic(fp(*op)(fp), i32 count, ...);
API fp fp_array_op_cumm_variadic(fp(*op)(fp, fp), i32 count, ...);


#endif //FIXEDPHYSICS_FIXEDMATH_H
