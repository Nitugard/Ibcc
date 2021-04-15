/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

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

#endif //PLATFORM_H
