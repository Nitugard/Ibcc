/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_GLMATH_H
#define IBCWEB_GLMATH_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

#ifdef _MSC_VER
#define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define INTERNAL_EXPAND(x) x
#define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#else

#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif

#define GL_PI 3.14159265359
#define GL_PI2 6.28318530718
#define GL_E 2.71828182846

#define GL_HALF 0.5f
#define GL_THIRD 0.333333333f
#define GL_RCP_180 0.0055555555f
#define GL_RCP_PI 0.31830988618

#define GL_VEC3_ZERO { .data= {0,0,0} }
#define GL_VEC3_ONE { .data= {1,1,1} }
#define GL_VEC3_LEFT { .data= {-1,0,0} }
#define GL_VEC3_RIGHT { .data= {1,0,0} }
#define GL_VEC3_UP { .data= {0,1,0} }
#define GL_VEC3_DOWN { .data= {0,-1,0} }
#define GL_VEC3_FWD { .data= {0,0,1} }
#define GL_VEC3_BACK { .data= {0,0,-1} }

#define GL_MAT_ZERO { .data= {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}}
#define GL_MAT_IDENTITY { .data= {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}}
#define GL_MAT_MUL_LR(...) gl_mat_op_cumm_variadic(gl_mat_mul, GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

typedef float gl_t;

typedef struct gl_vec2{
    union {
        gl_t data[2];
        struct {
            gl_t x, y;
        };
    };
} gl_vec2;

typedef struct gl_vec3{
    union {
        gl_t data[3];
        struct {
            gl_t x, y, z;
        };
    };
} gl_vec3;

typedef struct gl_vec4{
    union {
        gl_t data[4];
        struct {
            gl_t x, y, z, w;
        };
    };
} gl_vec4;

typedef struct gl_mat{

    union{
        gl_t data[16];
        struct
        {
            gl_t m00, m01, m02, m03;
            gl_t m10, m11, m12, m13;
            gl_t m20, m21, m22, m23;
            gl_t m30, m31, m32, m33;
        };
        gl_vec4 rows[4]; //rows
    };
} gl_mat;


/*
 * Functions
 */

API gl_t gl_abs(gl_t a);
API gl_t gl_sqrt(gl_t a);
API int32_t gl_sign(gl_t a);
API int32_t gl_sign_diff(gl_t a, gl_t b);
API gl_t gl_min(gl_t a, gl_t b);
API gl_t gl_max(gl_t a, gl_t b);
API gl_t gl_deg2rad(gl_t a);
API gl_t gl_rad2deg(gl_t a);
API gl_t gl_round(gl_t a);
API gl_t gl_floor(gl_t a);
API gl_t gl_fract(gl_t a);
API gl_t gl_ceil(gl_t a);
API gl_t gl_clamp(gl_t a, gl_t min, gl_t max);

/*
 * Trig functions
 */

API gl_t gl_sin(gl_t a);
API gl_t gl_cos(gl_t a);
API gl_t gl_tan(gl_t a);

API gl_t gl_asin(gl_t a);
API gl_t gl_acos(gl_t a);
API gl_t gl_atan(gl_t a);
API gl_t gl_atan2(gl_t a, gl_t b);

/*
 * Array operators
 */

API gl_t gl_array_op_cumm(gl_t const *a, gl_t (*op)(gl_t, gl_t), int32_t count);
API void gl_array_op(gl_t (*op)(gl_t), int32_t count, gl_t *array);

API void gl_array_op_variadic(gl_t(*op)(gl_t), int32_t count, ...);
API gl_t gl_array_op_cugl_variadic(gl_t(*op)(gl_t, gl_t), int32_t count, ...);


/*
 * Initializers
 */

API gl_vec3 gl_vec3_new(gl_t x, gl_t y, gl_t z);
API gl_vec3 gl_vec3_new_scalar(gl_t x);
API gl_vec3 gl_vec3_new_arr(float arr[3]);

API gl_vec4 gl_vec4_new(gl_t x, gl_t y, gl_t z, gl_t w);
API gl_vec4 gl_vec4_new_arr(float arr[4]);


/*
 * Operators
 */

API void gl_vec_add(gl_t const *a, gl_t const *b, int32_t count, gl_t *out);
API void gl_vec_sub(gl_t const *a, gl_t const *b, int32_t count, gl_t *out);
API void gl_vec_mul(gl_t const *a, gl_t const *b, int32_t count, gl_t *out);
API void gl_vec_div(gl_t const *a, gl_t const *b, int32_t count, gl_t *out);

/*
 * Functions
 */

API gl_t gl_vec_dot(gl_t const *a, gl_t const *b, int32_t count);
API gl_t gl_vec_norm_squared(gl_t const *a, int32_t count);
API gl_t gl_vec_norm(gl_t const *a, int32_t count);
API void gl_vec_normalize(gl_t *a, int32_t count);
API void gl_vec_negate(gl_t *a, uint32_t count);

API gl_vec3 gl_vec3_cross(gl_vec3 a, gl_vec3 b);
API gl_t gl_vec3_angle(gl_vec3 a, gl_vec3 b);
API gl_vec3 gl_vec3_normalize(gl_vec3 a);
API gl_t gl_vec3_norm(gl_vec3 a);
API gl_vec3 gl_vec3_norm_squared(gl_vec3 a);
API gl_t gl_vec3_dot(gl_vec3 a, gl_vec3 b);

API gl_vec3 gl_vec3_add(gl_vec3 a, gl_vec3 b);
API gl_vec3 gl_vec3_sub(gl_vec3 a, gl_vec3 b);
API gl_vec3 gl_vec3_mul(gl_vec3 a, gl_vec3 b);
API gl_vec3 gl_vec3_div(gl_vec3 a, gl_vec3 b);


/*
 * Matrix functions.
 */

API gl_mat gl_mat_new();
API gl_mat gl_mat_new_identity();
API gl_mat gl_mat_new_array(float arr[16]);

API gl_mat gl_mat_mul(gl_mat a, gl_mat b);
API gl_mat gl_mat_mul_p(gl_mat a, gl_t b);
API gl_vec3 gl_mat_mul_vec(gl_mat a, gl_vec3 b);
API gl_mat gl_mat_add(gl_mat a, gl_mat b);

API gl_vec4 gl_mat_column_get(gl_mat const *a, int32_t column);
API gl_vec4 gl_mat_row_get(gl_mat const *a, int32_t row);
API void gl_mat_column_set(gl_vec4 const *a, int32_t column, gl_mat *b);
API void gl_mat_row_set(gl_vec4 const *a, int32_t row, gl_mat *b);

API gl_mat gl_mat_trs(gl_vec3 translate, gl_vec3 rotate, gl_vec3 scale);
API gl_mat gl_mat_translate(gl_vec3 pos);
API gl_mat gl_mat_remove_translation(gl_mat m);
API gl_mat  gl_mat_set_translation(gl_mat m, struct gl_vec3 v);
API gl_vec3 gl_mat_get_translation(gl_mat mat);
API gl_mat gl_mat_rotate_x(gl_t angle);
API gl_mat gl_mat_rotate_y(gl_t angle);
API gl_mat gl_mat_rotate_z(gl_t angle);
API gl_mat gl_mat_rotate_zxy(gl_vec3 euler_angles);
API gl_mat gl_mat_perspective(gl_t fov, gl_t ar, gl_t near, gl_t far);
API gl_mat gl_mat_ortographic(gl_t bot, gl_t top, gl_t left, gl_t right, gl_t near, gl_t far);
API gl_mat gl_mat_look_at(gl_vec3 center, gl_vec3 eye, gl_vec3 up);
API gl_mat gl_mat_scale(gl_vec3 scale);
API gl_mat gl_mat_skew(gl_vec3 a);

API gl_t gl_mat_det(gl_mat a);
API gl_mat gl_mat_inverse(gl_mat a);
API gl_mat gl_mat_transpose(gl_mat a);
API gl_mat gl_mat_from_quaternion(gl_vec4 quat);

API gl_mat gl_mat_op_cumm(gl_mat* a, gl_mat (*op)(gl_mat const, gl_mat const), int32_t count);
API gl_mat gl_mat_op_cumm_variadic(gl_mat(*op)(gl_mat const, gl_mat const), int32_t count, ...);


API gl_vec4 gl_quat_from_axis_angle(gl_vec3 axis, float angle);
API gl_vec3 gl_quat_to_euler_angle(gl_vec4 q);
API gl_vec4 gl_euler_to_angle_quat(gl_vec3 euler) ;
API gl_vec4 gl_quat_conjugate(gl_vec4 q);
API gl_vec4 gl_quat_normalize(gl_vec4 q);
API gl_vec4 gl_quat_mul(gl_vec4 q1, gl_vec4 q2);
API gl_vec3 gl_quat_mul_vector(gl_vec4 q, gl_vec3 v);


/*
 * Print
 */

API void gl_print_vec(float* data, int rows, int cols);

#endif //IBCWEB_GLMATH_H
