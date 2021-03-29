/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Common.h"

#ifndef FIXEDPHYSICS_LOG_H
#define FIXEDPHYSICS_LOG_H

#define DEFAULT_IO

#define LOG_LEVEL_TRACE 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_WARN 8
#define LOG_LEVEL_ERROR 16
#define LOG_LEVEL_FATAL 32
#define LOG_LEVEL_ALL ((1 << 31) - 1)

#define LOG_TRACE(...) log_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

//todo: bit masking for each of these fields
i8 const* log_level_to_string(i32 level);
void log_set_level(i32 level);
void log_set_io(void *fp, i32 level);
void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif //FIXEDPHYSICS_LOG_H
