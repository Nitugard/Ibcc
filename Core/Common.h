/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef FIXEDPHYSICS_COMMON_H
#define FIXEDPHYSICS_COMMON_H

#include <stdbool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define P_WIN
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#define P_LINUX
#endif

#if defined(__linux__) && defined(__ANDROID__)
#define P_ANDROID
#endif

#if defined(_MSC_VER)
#define C_MSVC
#elif defined(__clang__)
#define C_CLANG
#elif defined(__GNUC__)
#define C_GNUC
#endif

#ifndef API
#define API extern
#endif

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;
typedef double f64;
typedef float f32;

typedef long long i64;
typedef int i32;
typedef char i8;

typedef long int ptrdiff;

#endif //FIXEDPHYSICS_COMMON_H
