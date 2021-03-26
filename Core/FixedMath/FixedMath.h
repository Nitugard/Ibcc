/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

 //Todo: Compare

#ifndef FIXEDPHYSICS_FIXEDMATH_H
#define FIXEDPHYSICS_FIXEDMATH_H

#include "../Types.h"

typedef i64 fp;
typedef struct fp_vec3 fp_vec3;
typedef struct fp_vec4 fp_vec4;
typedef struct fp_mat4 fp_mat4;

#define DEBUG
//#define FLOAT_MATH


#ifndef FP_ASSERT
#include <assert.h>
#define FP_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

#ifndef API
#define API extern
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
#define FP_ADD(...) fp_op2_p_va(fp_add_p, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_SUB(...) fp_op2_p_va(fp_sub_p, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_MUL(...) fp_op2_p_va(fp_mul_p, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_DIV(...) fp_op2_p_va(fp_div_p, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define FP_NEG(V) fp_negate_p(V)
#define FP_MUL_N(...) FP_NEG(FP_MUL(__VA_ARGS__))

#define FP_OP2_VA(OP2, ...) fp_op2_p_va(OP2, NARGS(__VA_ARGS__), __VA_ARGS__)

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
#define FP_PI3O2 (23860929LL)
#define FP_DEG2RAD (74961321LL)
#define FP_RAD2DEG (246083499208LL)

#define FP_VEC3_ZERO { .data= 0,0,0} }
#define FP_VEC3_ONE { .data= {FP_ONE,FP_ONE,FP_ONE} }
#define FP_VEC3_LEFT { .data= {FP_NEG_ONE,0,0} }
#define FP_VEC3_RIGHT { .data= {FP_ONE,0,0} }
#define FP_VEC3_UP { .data= {0,FP_ONE,0} }
#define FP_VEC3_DOWN { .data= {0,FP_NEG_ONE,0} }
#define FP_VEC3_FWD { .data= {0,0,FP_ONE} }
#define FP_VEC3_BACK { .data= {0,0,FP_NEG_ONE} }

#define FP_MAT_ZERO { .data= {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}}
#define FP_MAT_IDENTITY { .data= {FP_ONE,0,0,0, 0,FP_ONE,0,0, 0,0,FP_ONE,0, 0,0,0,FP_ONE}}

#define FP_QUAT_IDENTITY { .data= {0,0,0,FP_ONE}}

struct fp_vec3{
    union {
        fp data[3];
        struct {
            fp x, y, z;
        };
    };
};

struct fp_vec4{
    union {
        fp data[4];
        struct {
            fp x, y, z, w;
        };
    };
};

struct fp_mat4{

    union{
        fp data[16];
        struct
        {
            fp m00, m01, m02, m03;
            fp m10, m11, m12, m13;
            fp m20, m21, m22, m23;
            fp m30, m31, m32, m33;
        };
    };
};


API fp fp_from_int(i32);
API fp fp_from_double(f64);
API fp fp_from_uninit_fp(fp);
API i32 fp_to_int(fp);
API f64 fp_to_double(fp);

API fp fp_abs_p(fp);
API fp fp_sqrt_p(fp);

API i32 fp_sign_p(fp);
API i32 fp_signdiff_p(fp, fp);

API fp fp_add_p(fp, fp);
API fp fp_sub_p(fp, fp);
API fp fp_mul_p(fp, fp);
API fp fp_div_p(fp, fp);
API i32 fp_compare_p(fp, fp);
API i32 fp_less_p(fp, fp);
API i32 fp_greater_p(fp, fp);
API i32 fp_equal_p(fp, fp);
API i32 fp_not_eq_p(fp, fp);
API i32 fp_less_or_eq_p(fp, fp);
API i32 fp_greater_or_eq_p(fp, fp) ;

API fp fp_mod_p(fp, fp);
API fp fp_negate_p(fp);

API fp fp_sin_p(fp);
API fp fp_cos_p(fp);
API fp fp_tan_p(fp);

API fp fp_asin_p(fp);
API fp fp_acos_p(fp);
API fp fp_atan_p(fp);
API fp fp_atan2_p(fp, fp);

API fp fp_min_p(fp, fp);
API fp fp_max_p(fp, fp);

API fp fp_deg2rad_p(fp);
API fp fp_rad2deg_p(fp);

API fp fp_round_p(fp);
API fp fp_floor_p(fp);
API fp fp_fract_p(fp);
API fp fp_ceil_p(fp);


API fp fp_op2_v(fp const*, fp(*)(fp, fp), i32);
API void fp_op_v(fp(*)(fp), i32, fp*);

API void fp_op_p_va(fp(*op)(fp), i32 count, ...);
API fp fp_op2_p_va(fp(*op)(fp, fp), i32 count, ...);

API void fp_add_v(fp const*, fp const*, i32, fp*);
API void fp_sub_v(fp const*, fp const*, i32, fp*);
API void fp_mul_v(fp const*, fp const*, i32, fp*);
API void fp_div_v(fp const*, fp const*, i32, fp*);

API fp fp_dot_v(fp const*, fp const*, i32);
API fp fp_norm_sqr_v(fp const*, i32);
API fp fp_norm_v(fp const*, i32);
API void fp_normalize_v(i32, fp *);

API fp_vec3 fp_cross_v(const fp_vec3 *, const fp_vec3*);
API fp fp_angle_v(fp_vec3 const*, fp_vec3 const*);

API fp_mat4 fp_mul_m(fp_mat4 const*, fp_mat4 const*);
API fp_mat4 fp_mul_mp(fp_mat4 const*, fp);
API fp_mat4 fp_add_m(fp_mat4 const*, fp_mat4 const*);

API fp_vec4 fp_col_m_get(fp_mat4 const*, i32);
API fp_vec4 fp_row_m_get(fp_mat4 const*, i32);
API void fp_col_m_set(fp_vec4 const *, i32, fp_mat4 *);
API void fp_row_m_set(fp_vec4 const*, i32, fp_mat4 *);

API fp_mat4 fp_trs_m(fp_vec3 const*, fp_vec3 const*, fp_vec3 const*);
API fp_mat4 fp_translate_m(fp_vec3 const*);
API fp_mat4 fp_rotateX_m(fp angle);
API fp_mat4 fp_rotateY_m(fp angle);
API fp_mat4 fp_rotateZ_m(fp angle);
API fp_mat4 fp_rotateZXY_m(fp_vec3 const*);
API fp_mat4 fp_axis_angle_m(const fp_vec3 *, fp);
API fp_mat4 fp_scale_m(fp_vec3 const*);
API fp_mat4 fp_skew_m(const fp_vec3* v);

API fp fp_det_m(fp_mat4 const*);
API fp_mat4 fp_inv_m(fp_mat4 const*);

#endif //FIXEDPHYSICS_FIXEDMATH_H
