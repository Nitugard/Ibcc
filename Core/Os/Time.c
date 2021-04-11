/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Time.h"
#include <time.h>
#include <sys/stat.h>

i64 time_file_created(const i8* fp) {
    struct stat t_stat;
    stat(fp, &t_stat);
    return t_stat.st_ctime;
}

i64 time_file_modified(const i8 * fp) {
    struct stat t_stat;
    stat(fp, &t_stat);
    return t_stat.st_mtime;
}

i64 time_now() {
    time_t t = time(0);
    return t;
}

bool time_equals(i64 a, i64 b) {
    return a == b;
}
