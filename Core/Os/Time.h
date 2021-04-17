/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef TIME_H
#define TIME_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif


API int64_t time_now();
API bool time_equals(int64_t, int64_t);

API int64_t time_file_created(char const*);
API int64_t time_file_modified(char const*);

#endif //TIME_H
