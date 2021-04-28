/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef LOG_H
#define LOG_H

/*
 * Whether to output to default stream, currently it is redundant.
 * Todo: more streams
 */
#define DEFAULT_IO
//#define LOG_VERBOSE

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

#define LOG_LEVEL_TRACE 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_WARN 8
#define LOG_LEVEL_ERROR 16
#define LOG_LEVEL_FATAL 32

#define LOG_TRACE(...) log_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

API char const* log_level_to_string(int32_t level);
API void log_set_level(int32_t level);
API void log_set_io(void *fp, int32_t level);
API void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif //LOG_H
