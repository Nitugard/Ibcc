/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Log.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include <windows.h>
#include <math.h>

#define COLOR(text, background) ((text) + (background) * 16)
#define BLACK 0
#define GRAY 8
#define BLUE 1
#define LBLUE 9
#define GREEN 2
#define RED 4
#define PURPLE 5
#define YELLOW 6
#define WHITE 7
int32_t colors[] = {COLOR(WHITE, BLACK),
                    COLOR(LBLUE, BLACK),
                    COLOR(GREEN, BLACK),
                    COLOR(PURPLE, BLACK),
                    COLOR(RED, BLACK),
                    COLOR(RED, BLACK)};


static void print(struct _iobuf* io, const char* data, const char* file, int32_t line, int32_t level) {
    char buf[16];
    time_t time_now = time(NULL);
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&time_now))] = '\0';
    fprintf(io, "%s %-5s %s:%d: %s", buf, log_level_to_string(level), file, line, data);
    fflush(io);
}

void log_log(int32_t level, const char *file, int32_t line, const char *fmt, ...) {
    HANDLE hndl;
    hndl = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hndl);
    SetConsoleTextAttribute(hndl, colors[(int32_t)log2(level)]);

    static char buffer[1024*1024];
    va_list list;
    va_start(list, fmt);
    vsprintf(buffer, fmt, list);
    va_end(list);
#ifdef DEFAULT_IO
    print(stdout, buffer, file, line, level);
#endif
    SetConsoleTextAttribute(hndl, 15);
}

char const *log_level_to_string(int32_t level) {
    switch (level) {

        case LOG_LEVEL_TRACE: return "TRACE";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_WARN: return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}
