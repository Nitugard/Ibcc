/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MATH_H
#define IBC_MATH_H


#include <stdbool.h>
#include <stdint.h>


#ifndef API
#define API
#endif

#ifdef _MSC_VER // Microsoft compilers

#   define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#   define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define INTERNAL_EXPAND(x) x
#   define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#else // Non-Microsoft compilers

#   define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#   define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif
#define MM_ADD(...) mm_array_op_cumm_variadic(mm_add, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define MM_SUB(...) mm_array_op_cumm_variadic(mm_sub, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define MM_MUL(...) mm_array_op_cumm_variadic(mm_mul, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define MM_DIV(...) mm_array_op_cumm_variadic(mm_div, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define MM_NEG(V) mm_negate(V)
#define MM_MUL_N(...) MM_NEG(MM_MUL(__VA_ARGS__))
#define MM_OP2_VA(OP2, ...) mm_array_op_cumm_variadic(OP2, NARGS(__VA_ARGS__), __VA_ARGS__)

//IEEE754
#define MM_PI 3.14159265359
#define MM_PI2 6.28318530718
#define MM_E 2.71828182846

#define MM_ONE 1.0f
#define MM_180 180.0f
#define MM_HALF 0.5f
#define MM_THIRD (0.333333333f)
#define MM_RCP_180 (0.0055555555f)
#define MM_RCP_PI (0.31830988618)
#define MM_ZERO 0

#define MM_VEC3_ZERO { .data= {MM_ZERO,MM_ZERO,MM_ZERO} }
#define MM_VEC3_ONE { .data= {MM_ONE,MM_ONE,MM_ONE} }
#define MM_VEC3_LEFT { .data= {MM_NEG_ONE,MM_ZERO,MM_ZERO} }
#define MM_VEC3_RIGHT { .data= {MM_ONE,MM_ZERO,MM_ZERO} }
#define MM_VEC3_UP { .data= {MM_ZERO,MM_ONE,MM_ZERO} }
#define MM_VEC3_DOWN { .data= {MM_ZERO,MM_NEG_ONE,MM_ZERO} }
#define MM_VEC3_FWD { .data= {MM_ZERO,MM_ZERO,MM_ONE} }
#define MM_VEC3_BACK { .data= {MM_ZERO,MM_ZERO,MM_NEG_ONE} }

#define MM_MAT_ZERO { .data= {MM_ZERO,MM_ZERO,MM_ZERO,MM_ZERO, MM_ZERO,MM_ZERO,MM_ZERO,MM_ZERO, MM_ZERO,MM_ZERO,MM_ZERO,MM_ZERO, MM_ZERO,MM_ZERO,MM_ZERO,MM_ZERO}}
#define MM_MAT_IDENTITY { .data= {MM_ONE,MM_ZERO,MM_ZERO,MM_ZERO, MM_ZERO,MM_ONE,MM_ZERO,MM_ZERO, MM_ZERO,MM_ZERO,MM_ONE,MM_ZERO, MM_ZERO,MM_ZERO,MM_ZERO,MM_ONE}}
#define MM_MAT_MUL(...) mm_mat_op_cumm_variadic(mm_mat_mul, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)


typedef double mm_t;

typedef struct mm_vec3{
    union {
        mm_t data[3];
        struct {
            mm_t x, y, z;
        };
    };
} mm_vec3;

typedef struct mm_vec4{
    union {
        mm_t data[4];
        struct {
            mm_t x, y, z, w;
        };
    };
} mm_vec4;

typedef struct mm_mat4{

    union{
        mm_t data[16];
        struct
        {
            mm_t m00, m01, m02, m03;
            mm_t m10, m11, m12, m13;
            mm_t m20, m21, m22, m23;
            mm_t m30, m31, m32, m33;
        };
        mm_vec4 data_vec[4];
    };
} mm_mat4;


API mm_t mm_new(uint64_t data);
API mm_t mm_new_fraction(int64_t a, int64_t b);
API mm_t mm_from_raw(int64_t a);

API int64_t mm_int(mm_t a);
API float mm_float(mm_t a);
API double mm_double(mm_t a);

/*
 * Conversion functions operating on arrays, convenient for opengl.
 */

API void mm_array_to_int_array(mm_t const* array, int32_t count, int32_t* out);
API void mm_array_to_float_array(mm_t const* array, int32_t count, float* out);
API void mm_array_to_double_array(mm_t const* array, int32_t count, double* out);


/*
 * Functions
 */

API mm_t mm_abs(mm_t a);
API mm_t mm_sqrt(mm_t a);
API int32_t mm_sign(mm_t a);
API int32_t mm_sign_diff(mm_t a, mm_t b);
API mm_t mm_negate(mm_t a);
API mm_t mm_min(mm_t a, mm_t b);
API mm_t mm_max(mm_t a, mm_t b);
API mm_t mm_deg2rad(mm_t a);
API mm_t mm_rad2deg(mm_t a);
API mm_t mm_round(mm_t a);
API mm_t mm_floor(mm_t a);
API mm_t mm_fract(mm_t a);
API mm_t mm_ceil(mm_t a);

/*
 * Operators
 */

API mm_t mm_add(mm_t a, mm_t b);
API mm_t mm_sub(mm_t a, mm_t b);
API mm_t mm_mul(mm_t a, mm_t b);
API mm_t mm_div(mm_t a, mm_t b);

/*
 * Logic operators
 */

API int32_t mm_compare(mm_t a, mm_t b);
API bool mm_less_equal(mm_t a, mm_t b);
API bool mm_less(mm_t a, mm_t b);
API bool mm_equal(mm_t a, mm_t b);

/*
 * Trig functions
 */

API mm_t mm_sin(mm_t a);
API mm_t mm_cos(mm_t a);
API mm_t mm_tan(mm_t a);

API mm_t mm_asin(mm_t a);
API mm_t mm_acos(mm_t a);
API mm_t mm_atan(mm_t a);
API mm_t mm_atan2(mm_t a, mm_t b);

/*
 * Array operators
 */

API mm_t mm_array_op_cumm(mm_t const *a, mm_t (*op)(mm_t, mm_t), int32_t count);
API void mm_array_op(mm_t (*op)(mm_t), int32_t count, mm_t *array);

API void mm_array_op_variadic(mm_t(*op)(mm_t), int32_t count, ...);
API mm_t mm_array_op_cumm_variadic(mm_t(*op)(mm_t, mm_t), int32_t count, ...);


/*
 * Initializers
 */

API mm_vec3 mm_vec3_new(mm_t x, mm_t y, mm_t z);
API mm_vec3 mm_vec3_new_scalar(mm_t x);
API mm_vec4 mm_vec4_new(mm_t x, mm_t y, mm_t z, mm_t w);


/*
 * Operators
 */

API void mm_vec_add(mm_t const *a, mm_t const *b, int32_t count, mm_t *out);
API void mm_vec_sub(mm_t const *a, mm_t const *b, int32_t count, mm_t *out);
API void mm_vec_mul(mm_t const *a, mm_t const *b, int32_t count, mm_t *out);
API void mm_vec_div(mm_t const *a, mm_t const *b, int32_t count, mm_t *out);

/*
 * Functions
 */

API mm_t mm_vec_dot(mm_t const *a, mm_t const *b, int32_t count);
API mm_t mm_vec_norm_squared(mm_t const *a, int32_t count);
API mm_t mm_vec_norm(mm_t const *a, int32_t count);
API void mm_vec_normalize(mm_t *a, int32_t count);
API void mm_vec_negate(mm_t *a, uint32_t count);

API mm_vec3 mm_vec3_cross(mm_vec3 a, mm_vec3 b);
API mm_t mm_vec3_angle(mm_vec3 a, mm_vec3 b);
API mm_vec3 mm_vec3_normalize(mm_vec3 a);
API mm_vec3 mm_vec3_norm(mm_vec3 a);
API mm_vec3 mm_vec3_norm_squared(mm_vec3 a);
API mm_t mm_vec3_dot(mm_vec3 a, mm_vec3 b);

API mm_vec3 mm_vec3_add(mm_vec3 a, mm_vec3 b);
API mm_vec3 mm_vec3_sub(mm_vec3 a, mm_vec3 b);
API mm_vec3 mm_vec3_mul(mm_vec3 a, mm_vec3 b);
API mm_vec3 mm_vec3_div(mm_vec3 a, mm_vec3 b);

API mm_mat4 mm_mat_mul(mm_mat4 const *a, mm_mat4 const *b);
API mm_mat4 mm_mat_mul_p(mm_mat4 const *a, mm_t b);
API mm_vec3 mm_mat_mul_vec(mm_mat4 const *a, mm_vec3 const* b);
API mm_mat4 mm_mat_add(mm_mat4 const *a, mm_mat4 const *b);

API mm_vec4 mm_mat_column_get(mm_mat4 const *a, int32_t column);
API mm_vec4 mm_mat_row_get(mm_mat4 const *a, int32_t row);
API void mm_mat_column_set(mm_vec4 const *a, int32_t column, mm_mat4 *b);
API void mm_mat_row_set(mm_vec4 const *a, int32_t row, mm_mat4 *b);

API mm_mat4 mm_mat_trs(mm_vec3 const *translate, mm_vec3 const *rotate, mm_vec3 const *scale);
API mm_mat4 mm_mat_translate(mm_vec3 const *pos);
API mm_mat4 mm_mat_rotate_x(mm_t angle);
API mm_mat4 mm_mat_rotate_y(mm_t angle);
API mm_mat4 mm_mat_rotate_z(mm_t angle);
API mm_mat4 mm_mat_rotate_zxy(mm_vec3 const *euler_angles);
API mm_mat4 mm_mat_axis_angle(const mm_vec3 *axis, mm_t angle);
API mm_mat4 mm_mat_perspective(mm_t fov, mm_t ar, mm_t near, mm_t far);
API mm_mat4 mm_mat_ortographic(mm_t bot, mm_t top, mm_t left, mm_t right, mm_t near, mm_t far);
API mm_mat4 mm_mat_look_at(mm_vec3 center, mm_vec3 eye, mm_vec3 up);
API mm_mat4 mm_mat_scale(mm_vec3 const *scale);
API mm_mat4 mm_mat_skew(const mm_vec3* a);

API mm_t mm_mat_det(mm_mat4 const *a);
API mm_mat4 mm_mat_inverse(mm_mat4 const *a);
API mm_mat4 mm_mat_transpose(mm_mat4 const *a);

API mm_mat4 mm_mat_op_cumm(mm_mat4 const *a, mm_mat4 (*op)(mm_mat4 const*, mm_mat4 const*), int32_t count);
API mm_mat4 mm_mat_op_cumm_variadic(mm_mat4(*op)(mm_mat4 const*, mm_mat4 const*), int32_t count, ...);


#endif //IBC_MATH_H
