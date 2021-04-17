/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Time.h"
#include <time.h>
#include <sys/stat.h>

int64_t time_file_created(const char* fp) {
    struct stat t_stat;
    stat(fp, &t_stat);
    return t_stat.st_ctime;
}

int64_t time_file_modified(const char * fp) {
    struct stat t_stat;
    stat(fp, &t_stat);
    return t_stat.st_mtime;
}

int64_t time_now() {
    time_t t = time(0);
    return t;
}

bool time_equals(int64_t a, int64_t b) {
    return a == b;
}
