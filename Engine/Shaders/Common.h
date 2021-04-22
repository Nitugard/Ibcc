/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_COMMON_H
#define IBC_COMMON_H



#define position_attr 0
#define color_attr 1
#define uv_attr 2
#define normal_attr 3

#define SHADER_VERSION #version 300 es\n precision highp float;\n

#define SHADER_MATRICES \
    uniform mat4 model;                    \
    layout(std140, row_major) uniform matrices{ \
        mat4 view; \
        mat4 projection; \
        vec3 light_pos; \
        vec3 cam_pos; \
    };

typedef struct SH_MVP_T{
    float view[16];
    float projection[16];

    float light_pos[3];
    float __pad1;
    float cam_pos[3];
    float __pad2;
} SH_MVP_T;

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

#define PASTE(a, b) a ## b
#define XPASTE(a, b) PASTE(a,b)

#define APPLYX1(a)           X(a)
#define APPLYX2(a, b)         X(a) X(b)
#define APPLYX3(a, b, c)       X(a) X(b) X(c)
#define APPLYX4(a, b, c, d)     X(a) X(b) X(c) X(d)
#define APPLYX5(a, b, c, d, e)   X(a) X(b) X(c) X(d) X(e)
#define APPLYX6(a, b, c, d, e, f) X(a) X(b) X(c) X(d) X(e) X(f)
#define APPLYX7(a, b, c, d, e, f, g) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g)
#define APPLYX8(a, b, c, d, e, f, g, h) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h)
#define APPLYX9(a, b, c, d, e, f, g, h, i) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i)
#define APPLYX10(a, b, c, d, e, f, g, h, i, j) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j)
#define APPLYX11(a, b, c, d, e, f, g, h, i, j, k) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j) X(k)
#define APPLYX12(a, b, c, d, e, f, g, h, i, j, k, l) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j) X(k) X(l)
#define APPLYX13(a, b, c, d, e, f, g, h, i, j, k, l, m) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j) X(k) X(l) X(m)
#define APPLYX14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j) X(k) X(l) X(m) X(n)
#define APPLYX15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
    X(a) X(b) X(c) X(d) X(e) X(f) X(g) X(h) X(i) X(j) X(k) X(l) X(m) X(n) X(o)
#define APPLYX_(M, ...) M(__VA_ARGS__)
#define APPLYXn(...) APPLYX_(XPASTE(APPLYX, GET_ARG_COUNT(__VA_ARGS__)), __VA_ARGS__)

#define CONCATENATE(...) APPLYXn(__VA_ARGS__)
#define X(x) #x

#endif //IBC_COMMON_H
