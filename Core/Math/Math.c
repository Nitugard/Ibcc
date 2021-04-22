/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Math.h"
#include <math.h>
#include <stdarg.h>


#ifndef CORE_ASSERT
#ifdef __MINGW32__
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#else
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif
#endif


mm_t mm_abs(mm_t a) { if(a < 0) return -a; return a; }
mm_t mm_sqrt(mm_t a) { return sqrt(a); }
int32_t mm_sign(mm_t a) { if (a >= 0) return 1; return -1; }
int32_t mm_sign_diff(mm_t a, mm_t b) { return mm_sign(a) * mm_sign(b); }
mm_t mm_negate(mm_t a) { return -a; }
mm_t mm_min(mm_t a, mm_t b) { if (a > b) return b; return a; }
mm_t mm_max(mm_t a, mm_t b) { if (a > b) return a; return b; }
mm_t mm_deg2rad(mm_t a) { return a * MM_RCP_180 * MM_PI; }
mm_t mm_rad2deg(mm_t a){ return a * MM_180 * MM_RCP_PI; }
mm_t mm_round(mm_t a) { return round(a); }
mm_t mm_floor(mm_t a) { return floor(a); }
mm_t mm_ceil(mm_t a) { return ceil(a); }


mm_t mm_add(mm_t a, mm_t b) { return a + b; }
mm_t mm_sub(mm_t a, mm_t b) { return a - b; }
mm_t mm_mul(mm_t a, mm_t b) { return a * b; }
mm_t mm_div(mm_t a, mm_t b) { return a / b; }


int32_t mm_compare(mm_t a, mm_t b) { if(a > b) return 1; if(a<b) return -1; return 0;}
bool mm_less_equal(mm_t a, mm_t b) { return a <= b; }
bool mm_less(mm_t a, mm_t b) { return a < b;}
bool mm_equal(mm_t a, mm_t b) { return a == b;}


mm_t mm_sin(mm_t a) { return sin(a); }
mm_t mm_cos(mm_t a) { return cos(a); }
mm_t mm_tan(mm_t a) { return tan(a); }

mm_t mm_asin(mm_t a) { return asin(a); }
mm_t mm_acos(mm_t a) { return acos(a); }
mm_t mm_atan(mm_t a) { return atan(a); }
mm_t mm_atan2(mm_t a, mm_t b) { return atan2(a, b); }

mm_t mm_new(uint64_t data) {return data;}
mm_t mm_new_fraction(int64_t a, int64_t b) {return (float)a / b;}
mm_t mm_from_raw(int64_t a) {return a;}

int64_t mm_int(mm_t a) { return a;}
float mm_float(mm_t a) { return a;}
double mm_double(mm_t a){ return a;}

void mm_array_to_int_array(mm_t const* array, int32_t count, int32_t* out) {for(int32_t i=0; i<count; ++i) out[i] = array[i];}
void mm_array_to_float_array(mm_t const* array, int32_t count, float* out) {for(int32_t i=0; i<count; ++i) out[i] = array[i];}
void mm_array_to_double_array(mm_t const* array, int32_t count, double* out) {for(int32_t i=0; i<count; ++i) out[i] = array[i];}


mm_t mm_array_op_cumm(mm_t const *a, mm_t (*op)(mm_t, mm_t), int32_t count) {
    mm_t res = a[0];
    for (int32_t i = 1; i < count; ++i) res = op(res, a[i]);
    return res;
}

void mm_array_op(mm_t (*op)(mm_t), int32_t count, mm_t *array) {
    for (int32_t i = 0; i < count; ++i) array[i] = op(array[i]);
}

mm_t mm_array_op_cumm_variadic(mm_t(*op)(mm_t, mm_t), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    mm_t num = va_arg(args, mm_t);
    for (int32_t i = 1; i < count; ++i)
        num = op(num, va_arg(args, mm_t));
    va_end(args);
    return num;
}

void mm_array_op_variadic(mm_t (*op)(mm_t), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    for (int32_t i = 1; i < count; ++i) {
        mm_t *num = va_arg(args, mm_t*);
        *num = op(*num);
    }
    va_end(args);
}



mm_vec3 mm_vec3_new(mm_t x, mm_t y, mm_t z) {
    mm_vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

API mm_vec3 mm_vec3_new_scalar(mm_t x)
{
    mm_vec3 result;
    result.x = x;
    result.y = x;
    result.z = x;
    return result;
}

mm_vec4 mm_vec4_new(mm_t x, mm_t y, mm_t z, mm_t w) {
    mm_vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

void mm_vec_add(mm_t const *a, mm_t const *b, int32_t count, mm_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = mm_add(a[i], b[i]);
}

void mm_vec_sub(mm_t const *a, mm_t const *b, int32_t count, mm_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = mm_sub(a[i], b[i]);
}

void mm_vec_mul(mm_t const *a, mm_t const *b, int32_t count, mm_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = mm_mul(a[i], b[i]);
}

void mm_vec_div(mm_t const *a, mm_t const *b, int32_t count, mm_t *out) {
    for (int32_t i = 0; i < count; ++i) out[i] = mm_div(a[i], b[i]);
}


mm_t mm_vec_dot(mm_t const *a, mm_t const *b, int32_t count) {
    mm_t res = MM_ZERO;
    for (int32_t i = 0; i < count; ++i) res = mm_add(res, mm_mul(a[i], b[i]));
    return res;
}

mm_t mm_vec_norm_squared(mm_t const *a, int32_t count) {
    return mm_vec_dot(a, a, count);
}

mm_t mm_vec_norm(mm_t const *a, int32_t count) {
    return mm_sqrt(mm_vec_norm_squared(a, count));
}

void mm_vec_normalize(mm_t *a, int32_t count) {
    mm_t norm = mm_vec_norm(a, count);
    mm_t rcp = mm_div(MM_ONE, norm);
    for (int32_t i = 0; i < count; ++i) a[i] = mm_mul(a[i], rcp);
}

void mm_vec_negate(mm_t *a, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) a[i] = mm_negate(a[i]);
}

mm_vec3 mm_vec3_normalize(mm_vec3 a) {
    mm_vec_normalize(a.data, 3);
    return a;
}

mm_vec3 mm_vec3_norm(mm_vec3 a){
    mm_vec_norm(a.data, 3);
    return a;
}

mm_vec3 mm_vec3_norm_squared(mm_vec3 a){
    mm_vec_norm_squared(a.data, 3);
    return a;
}

mm_t mm_vec3_dot(mm_vec3 a, mm_vec3 b)
{
    return mm_vec_dot(a.data, b.data, 3);
}

mm_vec3 mm_vec3_add(mm_vec3 a, mm_vec3 b){ mm_vec3 r; mm_vec_add(a.data, b.data, 3, r.data); return r; }
mm_vec3 mm_vec3_sub(mm_vec3 a, mm_vec3 b){ mm_vec3 r; mm_vec_sub(a.data, b.data, 3, r.data); return r; }
mm_vec3 mm_vec3_mul(mm_vec3 a, mm_vec3 b){ mm_vec3 r; mm_vec_mul(a.data, b.data, 3, r.data); return r; }
mm_vec3 mm_vec3_div(mm_vec3 a, mm_vec3 b){ mm_vec3 r; mm_vec_div(a.data, b.data, 3, r.data); return r; }


mm_vec3 mm_vec3_cross(mm_vec3 a, mm_vec3 b) {
    mm_vec3 res = {
            .x = mm_sub(mm_mul(a.y, b.z), mm_mul(a.z, b.y)),
            .y = mm_sub(mm_mul(a.z, b.x), mm_mul(a.x, b.z)),
            .z = mm_sub(mm_mul(a.x, b.y), mm_mul(a.y, b.x))
    };
    return res;
}

mm_t mm_vec3_angle(mm_vec3 a, mm_vec3 b) {
    return mm_acos(mm_div(mm_vec_dot(a.data, b.data, 3),
                          mm_mul(mm_vec_norm(a.data, 3), mm_vec_norm(b.data, 3))));
}


mm_t mm_mat_det(mm_mat4 const *a) {

    return MM_ADD(
            MM_MUL(a->m03, a->m12, a->m21, a->m30), MM_MUL_N(a->m02, a->m13, a->m21, a->m30),
            MM_MUL_N(a->m03, a->m11, a->m22, a->m30), MM_MUL(a->m01, a->m13, a->m22, a->m30),
            MM_MUL(a->m02, a->m11, a->m23, a->m30), MM_MUL_N(a->m01, a->m12, a->m23, a->m30),
            MM_MUL_N(a->m03, a->m12, a->m20, a->m31), MM_MUL(a->m02, a->m13, a->m20, a->m31),
            MM_MUL(a->m03, a->m10, a->m22, a->m31), MM_MUL_N(a->m00, a->m13, a->m22, a->m31),
            MM_MUL_N(a->m02, a->m10, a->m23, a->m31), MM_MUL(a->m00, a->m12, a->m23, a->m31),
            MM_MUL(a->m03, a->m11, a->m20, a->m32), MM_MUL_N(a->m01, a->m13, a->m20, a->m32),
            MM_MUL_N(a->m03, a->m10, a->m21, a->m32), MM_MUL(a->m00, a->m13, a->m21, a->m32),
            MM_MUL(a->m01, a->m10, a->m23, a->m32), MM_MUL_N(a->m00, a->m11, a->m23, a->m32),
            MM_MUL_N(a->m02, a->m11, a->m20, a->m33), MM_MUL(a->m01, a->m12, a->m20, a->m33),
            MM_MUL(a->m02, a->m10, a->m21, a->m33), MM_MUL_N(a->m00, a->m12, a->m21, a->m33),
            MM_MUL_N(a->m01, a->m10, a->m22, a->m33), MM_MUL(a->m00, a->m11, a->m22, a->m33));

}

mm_mat4 mm_mat_inverse(mm_mat4 const *a) {
    const mm_t det = mm_mat_det(a);
    const mm_t rcp = MM_DIV(MM_ONE, det);
    CORE_ASSERT(!mm_equal(det, MM_ZERO));

    mm_t m00 = MM_ADD(MM_MUL(a->m12, a->m23, a->m31), MM_MUL_N(a->m13, a->m22, a->m31), MM_MUL(a->m13, a->m21, a->m32),
                    MM_MUL_N(a->m11, a->m23, a->m32), MM_MUL_N(a->m12, a->m21, a->m33), MM_MUL(a->m11, a->m22, a->m33));
    mm_t m01 = MM_ADD(MM_MUL(a->m03, a->m22, a->m31), MM_MUL_N(a->m02, a->m23, a->m31), MM_MUL_N(a->m03, a->m21, a->m32),
                    MM_MUL(a->m01, a->m23, a->m32), MM_MUL(a->m02, a->m21, a->m33), MM_MUL_N(a->m01, a->m22, a->m33));
    mm_t m02 = MM_ADD(MM_MUL(a->m02, a->m13, a->m31), MM_MUL_N(a->m03, a->m12, a->m31), MM_MUL(a->m03, a->m11, a->m32),
                    MM_MUL_N(a->m01, a->m13, a->m32), MM_MUL_N(a->m02, a->m11, a->m33), MM_MUL(a->m01, a->m12, a->m33));
    mm_t m03 = MM_ADD(MM_MUL(a->m03, a->m12, a->m21), MM_MUL_N(a->m02, a->m13, a->m21), MM_MUL_N(a->m03, a->m11, a->m22),
                    MM_MUL(a->m01, a->m13, a->m22), MM_MUL(a->m02, a->m11, a->m23), MM_MUL_N(a->m01, a->m12, a->m23));
    mm_t m10 = MM_ADD(MM_MUL(a->m13, a->m22, a->m30), MM_MUL_N(a->m12, a->m23, a->m30), MM_MUL_N(a->m13, a->m20, a->m32),
                    MM_MUL(a->m10, a->m23, a->m32), MM_MUL(a->m12, a->m20, a->m33), MM_MUL_N(a->m10, a->m22, a->m33));
    mm_t m11 = MM_ADD(MM_MUL(a->m02, a->m23, a->m30), MM_MUL_N(a->m03, a->m22, a->m30), MM_MUL(a->m03, a->m20, a->m32),
                    MM_MUL_N(a->m00, a->m23, a->m32), MM_MUL_N(a->m02, a->m20, a->m33), MM_MUL(a->m00, a->m22, a->m33));
    mm_t m12 = MM_ADD(MM_MUL(a->m03, a->m12, a->m30), MM_MUL_N(a->m02, a->m13, a->m30), MM_MUL_N(a->m03, a->m10, a->m32),
                    MM_MUL(a->m00, a->m13, a->m32), MM_MUL(a->m02, a->m10, a->m33), MM_MUL_N(a->m00, a->m12, a->m33));
    mm_t m13 = MM_ADD(MM_MUL(a->m02, a->m13, a->m20), MM_MUL_N(a->m03, a->m12, a->m20), MM_MUL(a->m03, a->m10, a->m22),
                    MM_MUL_N(a->m00, a->m13, a->m22), MM_MUL_N(a->m02, a->m10, a->m23), MM_MUL(a->m00, a->m12, a->m23));
    mm_t m20 = MM_ADD(MM_MUL(a->m11, a->m23, a->m30), MM_MUL_N(a->m13, a->m21, a->m30), MM_MUL(a->m13, a->m20, a->m31),
                    MM_MUL_N(a->m10, a->m23, a->m31), MM_MUL_N(a->m11, a->m20, a->m33), MM_MUL(a->m10, a->m21, a->m33));
    mm_t m21 = MM_ADD(MM_MUL(a->m03, a->m21, a->m30), MM_MUL_N(a->m01, a->m23, a->m30), MM_MUL_N(a->m03, a->m20, a->m31),
                    MM_MUL(a->m00, a->m23, a->m31), MM_MUL(a->m01, a->m20, a->m33), MM_MUL_N(a->m00, a->m21, a->m33));
    mm_t m22 = MM_ADD(MM_MUL(a->m01, a->m13, a->m30), MM_MUL_N(a->m03, a->m11, a->m30), MM_MUL(a->m03, a->m10, a->m31),
                    MM_MUL_N(a->m00, a->m13, a->m31), MM_MUL_N(a->m01, a->m10, a->m33), MM_MUL(a->m00, a->m11, a->m33));
    mm_t m23 = MM_ADD(MM_MUL(a->m03, a->m11, a->m20), MM_MUL_N(a->m01, a->m13, a->m20), MM_MUL_N(a->m03, a->m10, a->m21),
                    MM_MUL(a->m00, a->m13, a->m21), MM_MUL(a->m01, a->m10, a->m23), MM_MUL_N(a->m00, a->m11, a->m23));
    mm_t m30 = MM_ADD(MM_MUL(a->m12, a->m21, a->m30), MM_MUL_N(a->m11, a->m22, a->m30), MM_MUL_N(a->m12, a->m20, a->m31),
                    MM_MUL(a->m10, a->m22, a->m31), MM_MUL(a->m11, a->m20, a->m32), MM_MUL_N(a->m10, a->m21, a->m32));
    mm_t m31 = MM_ADD(MM_MUL(a->m01, a->m22, a->m30), MM_MUL_N(a->m02, a->m21, a->m30), MM_MUL(a->m02, a->m20, a->m31),
                    MM_MUL_N(a->m00, a->m22, a->m31), MM_MUL_N(a->m01, a->m20, a->m32), MM_MUL(a->m00, a->m21, a->m32));
    mm_t m32 = MM_ADD(MM_MUL(a->m02, a->m11, a->m30), MM_MUL_N(a->m01, a->m12, a->m30), MM_MUL_N(a->m02, a->m10, a->m31),
                    MM_MUL(a->m00, a->m12, a->m31), MM_MUL(a->m01, a->m10, a->m32), MM_MUL_N(a->m00, a->m11, a->m32));
    mm_t m33 = MM_ADD(MM_MUL(a->m01, a->m12, a->m20), MM_MUL_N(a->m02, a->m11, a->m20), MM_MUL(a->m02, a->m10, a->m21),
                    MM_MUL_N(a->m00, a->m12, a->m21), MM_MUL_N(a->m01, a->m10, a->m22), MM_MUL(a->m00, a->m11, a->m22));


    mm_mat4 res = {
            .data = {
                    m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33
            }
    };

    for (int32_t i = 0; i < 16; ++i) res.data[i] = MM_MUL(res.data[i], rcp);
    return res;
}

mm_mat4 mm_mat_translate(mm_vec3 const *pos) {
    mm_mat4 result = MM_MAT_IDENTITY;
    result.m03 = pos->x;
    result.m13 = pos->y;
    result.m23 = pos->z;
    return result;
}

mm_mat4 mm_mat_skew(const mm_vec3 *a) {
    mm_mat4 result = {.data =
            {
                    MM_ZERO, mm_negate(a->z), a->y,
                    a->z, MM_ZERO, mm_negate(a->x),
                    mm_negate(a->y), a->x, MM_ZERO
            }
    };
    return result;
}

mm_mat4 mm_mat_axis_angle(const mm_vec3 *axis, mm_t angle) {
    const mm_t s = mm_sin(angle);
    const mm_t c = mm_cos(angle);
    const mm_t t = MM_SUB(MM_ONE, c);
    mm_vec3 naxis = *axis;
    mm_vec_normalize(naxis.data, 3);
    const mm_mat4 skew = mm_mat_skew(&naxis);
    const mm_mat4 skew2 = mm_mat_mul(&skew, &skew);
    const mm_mat4 id = MM_MAT_IDENTITY;
    const mm_mat4 term1 = mm_mat_mul_p(&skew, s);
    const mm_mat4 term2 = mm_mat_mul_p(&skew2, t);
    const mm_mat4 term3 = mm_mat_add(&term1, &term2);
    return mm_mat_add(&id, &term3);
}

mm_mat4 mm_mat_scale(mm_vec3 const *scale) {
    mm_mat4 result = MM_MAT_IDENTITY;
    result.m00 = scale->x;
    result.m11 = scale->y;
    result.m22 = scale->z;
    return result;

}

mm_mat4 mm_mat_mul(mm_mat4 const *a, mm_mat4 const *b) {
    mm_mat4  result;
    for (int32_t i = 0; i < 4; ++i) {
        for (int32_t j = 0; j < 4; ++j) {
            result.data[i * 4 + j] = mm_vec_dot(mm_mat_row_get(a, i).data, mm_mat_column_get(b, j).data, 4);
        }
    }

    return result;
}

void mm_mat_column_set(mm_vec4 const *a, int32_t column, mm_mat4 *b) {
    for (int32_t i = 0; i < 4; ++i)
        b->data[column + i * 4] = a->data[i];
}

mm_vec4 mm_mat_column_get(mm_mat4 const *a, int32_t column) {
    mm_vec4  result;
    for (int32_t i = 0; i < 4; ++i)
        result.data[i] = a->data[column + i * 4];
    return result;
}

void mm_mat_row_set(mm_vec4 const *a, int32_t row, mm_mat4 *b) {
    for (int32_t i = 0; i < 4; ++i)
        b->data[(row * 4) + i] = a->data[i];
}

mm_vec4 mm_mat_row_get(mm_mat4 const *a, int32_t row) {
    mm_vec4  result;
    for (int32_t i = 0; i < 4; ++i)
        result.data[i] = a->data[(row * 4) + i];
    return result;
}

mm_mat4 mm_mat_add(mm_mat4 const *a, mm_mat4 const *b) {
    mm_mat4  result;
    for (int32_t i = 0; i < 16; ++i)
        result.data[i] = MM_ADD(a->data[i], b->data[i]);
    return result;
}

mm_mat4 mm_mat_mul_p(mm_mat4 const *a, mm_t b) {
    mm_mat4 result;
    for(int32_t i=0; i < 16; ++i)
        result.data[i] = MM_MUL(a->data[i], b);
    return result;
}

API mm_vec3 mm_mat_mul_vec(mm_mat4 const *a, mm_vec3 const* b)
{
    mm_vec4 res;
    mm_vec4 v4 = {.x = b->x, .y = b->y, .z = b->z, .w = 1};
    mm_vec4 temp;
    for(int32_t i=0; i < 4; ++i)
    {
        temp = mm_mat_row_get(a, i);
        res.data[i] = mm_vec_dot(temp.data, v4.data, 4);
    }
    return mm_vec3_new(res.x, res.y, res.z);
}

mm_mat4 mm_mat_trs(mm_vec3 const *translate, mm_vec3 const *rotate, mm_vec3 const *scale) {
    const mm_mat4 sm = mm_mat_scale(scale);
    const mm_mat4 tm = mm_mat_translate(translate);
    const mm_mat4 rm = mm_mat_rotate_zxy(rotate);
    const mm_mat4 ts = mm_mat_mul(&tm, &sm);
    const mm_mat4 trs = mm_mat_mul(&rm, &ts);
    return trs;
}

mm_mat4 mm_mat_rotate_x(mm_t angle) {
    angle = mm_deg2rad(angle);
    const mm_t c = mm_cos(angle);
    const mm_t s = mm_sin(angle);
    mm_mat4 result = {.data = {
            MM_ONE,	MM_ZERO,	MM_ZERO, MM_ZERO,
            MM_ZERO,	c, mm_negate(s), MM_ZERO,
            MM_ZERO,	s,	c, MM_ZERO,
            MM_ZERO,  MM_ZERO,  MM_ZERO,MM_ONE
    }};
    return result;
}

mm_mat4 mm_mat_rotate_y(mm_t angle) {
    angle = mm_deg2rad(angle);
    const mm_t c = mm_cos(angle);
    const mm_t s = mm_sin(angle);
    mm_mat4 result = {.data ={
            c, MM_ZERO, s, MM_ZERO,
            MM_ZERO, MM_ONE, MM_ZERO, MM_ZERO,
            mm_negate(s), MM_ZERO, c, MM_ZERO,
            MM_ZERO, MM_ZERO, MM_ZERO,MM_ONE
    }};

    return result;
}

mm_mat4 mm_mat_rotate_z(mm_t angle) {
    angle = mm_deg2rad(angle);
    const mm_t c = mm_cos(angle);
    const mm_t s = mm_sin(angle);
    mm_mat4 result = {.data ={
            c, mm_negate(s), MM_ZERO, MM_ZERO,
            s, c, MM_ZERO, MM_ZERO,
            MM_ZERO, MM_ZERO, MM_ONE, MM_ZERO,
            MM_ZERO, MM_ZERO, MM_ZERO, MM_ONE,
    }};
    return result;
}

mm_mat4 mm_mat_rotate_zxy(const mm_vec3 *euler_angles) {
    const mm_mat4 rz = mm_mat_rotate_z(euler_angles->z);
    const mm_mat4 rx = mm_mat_rotate_x(euler_angles->x);
    const mm_mat4 ry = mm_mat_rotate_y(euler_angles->y);
    const mm_mat4 zx = mm_mat_mul(&rx, &rz);
    const mm_mat4 zxy = mm_mat_mul(&ry, &zx);
    return zxy;
}

mm_mat4 mm_mat_perspective(mm_t fov, mm_t ar, mm_t near, mm_t far) {
    mm_t cot = MM_DIV(MM_ONE, mm_tan(MM_MUL(fov, MM_DIV(MM_PI, mm_new(360)))));
    struct mm_mat4 result = MM_MAT_ZERO;
    result.m00 = MM_DIV(cot, ar);
    result.m11 = cot;
    result.m32 = mm_negate(MM_ONE);
    result.m22 = MM_DIV(MM_ADD(near, far), MM_SUB(near, far));
    result.m23 = MM_DIV(MM_MUL(mm_new(2), near, far), MM_SUB(near, far));
    result.m33 = MM_ZERO;
    return result;
}

mm_mat4 mm_mat_transpose(mm_mat4 const *a) {
    struct mm_mat4 result;
    for(int32_t c = 0; c < 4; ++c) {
        for (int32_t r = 0; r < 4; ++r) {
            result.data[c + r * 4] = a->data[r + c * 4];
        }
    }

    return result;
}

mm_mat4 mm_mat_op_cumm(mm_mat4 const *a, mm_mat4 (*op)(mm_mat4 const*, mm_mat4 const*), int32_t count) {
    mm_mat4 res = a[0];
    for (int32_t i = 1; i < count; ++i) res = op(&res, &a[i]);
    return res;
}

mm_mat4 mm_mat_op_cumm_variadic(mm_mat4(*op)(mm_mat4 const*, mm_mat4 const*), int32_t count, ...) {
    va_list args;
    va_start(args, count);
    CORE_ASSERT(count > 1);
    mm_mat4* num = va_arg(args, mm_mat4*);
    for (int32_t i = 1; i < count; ++i) {
        *num = op(num, va_arg(args, mm_mat4*));
    }
    va_end(args);
    return *num;
}

mm_mat4 mm_mat_ortographic(mm_t bot, mm_t top, mm_t left, mm_t right, mm_t near, mm_t far) {

    mm_t two = mm_new(2);
    mm_mat4 result = MM_MAT_IDENTITY;
    result.m00 = MM_DIV(two, MM_SUB(right, left));
    result.m11 = MM_DIV(two, MM_SUB(top, bot));
    result.m22 = MM_DIV(two, MM_SUB(near, far));
    result.m33 = MM_ONE;
    result.m30 = MM_DIV(MM_ADD(left, right), MM_SUB(left, right));
    result.m31 = MM_DIV(MM_ADD(bot, top), MM_SUB(bot, top));
    result.m32 = MM_DIV(MM_ADD(far, near), MM_SUB(near, far));
    return result;
}

mm_mat4 mm_mat_look_at(mm_vec3 center, mm_vec3 eye, mm_vec3 up) {
    mm_vec3 zaxis = mm_vec3_normalize(mm_vec3_sub(center, eye));
    mm_vec3 xaxis = mm_vec3_normalize(mm_vec3_cross(zaxis, up));
    mm_vec3 yaxis = mm_vec3_cross(xaxis, zaxis);

    mm_vec_negate(zaxis.data, 3);

    mm_mat4 viewMatrix = {.data_vec = {
            mm_vec4_new(xaxis.x, xaxis.y, xaxis.z, -mm_vec3_dot(xaxis, eye)),
            mm_vec4_new(yaxis.x, yaxis.y, yaxis.z, -mm_vec3_dot(yaxis, eye)),
            mm_vec4_new(zaxis.x, zaxis.y, zaxis.z, -mm_vec3_dot(zaxis, eye)),
            mm_vec4_new(0, 0, 0, 1)
    }
    };

    return viewMatrix;
}