/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "String.h"
#include "Array.h"

#include <string.h> //memcpy, strlen

#ifndef CORE_ASSERT
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

/*
 * Important notes to ease future edits:
 * - String is placed within array that is null terminated
 * - String size is different from the internal array size by one
 * - Important! String and array sizes are one to one otherwise str_new_capacity would constantly resize it,
 * and maybe even other things depend on it(check usage of capacity).
 */

string_handle str_new(const char *str) {
    int32_t len = strlen(str);
    arr_handle handle = arr_new(sizeof(char), len+1);
    memcpy(arr_get(handle, 0), str, len + 1);
    arr_resize(handle, len+1);
    return handle;
}


string_handle str_new_capacity(int32_t capacity) {
    CORE_ASSERT(capacity >= 1);
    arr_handle handle = arr_new(sizeof(char), capacity);
    arr_resize(handle, capacity);
    return handle;
}

string_handle str_concat(string_handle base, const char *str) {
    int32_t len = strlen(str);
    string_handle res = str_new_capacity(arr_size(base) + len);
    memcpy((char*) arr_get(res, 0), arr_get(base, 0), str_get_size(base));
    memcpy((char*) arr_get(res, 0) + str_get_size(base), str, len + 1);
    return res;
}

string_handle str_new_handle(string_handle str) {
    return str_new(str_internal(str));
}

string_handle str_concat_handle(string_handle base, string_handle str) {
    return str_concat(base, str_internal(str));
}

string_handle str_substring(string_handle base, int32_t offset, int32_t length) {
    CORE_ASSERT(offset + length <= str_get_size(base));
    string_handle res = str_new_capacity(length + 1);
    memcpy((char*) arr_get(res, 0), ((char const*) arr_get(base, 0)) + offset, length);
    arr_set(res, length, "\0");
    return res;
}

int32_t str_get_size(string_handle handle) {
    return arr_size(handle) - 1;
}

int32_t str_find(string_handle handle, const char *str) {
    int32_t len = strlen(str);
    if(len == 0) return -1;
    int32_t count = 0;
    for(int32_t j=0; j < str_get_size(handle) - len + 1; ++j) {
        if (*(char*)arr_get(handle, j) == str[count]) {
            count++;
        }
        else count = 0;

        if(count == len)
            return j-count + 1;
    }
    return -1;
}

const char *str_internal(string_handle handle) {
    return (char*) arr_get(handle, 0);
}

int32_t str_cmp(const char *a, const char *b) {
    return strcmp(a, b);
}

int32_t str_find_last(string_handle handle, const char *str) {
    int32_t len = strlen(str);
    if (len == 0) return -1;
    int32_t count = 0;
    for (int32_t j = arr_size(handle) - len; j >= 0; --j) {
        while(*(char *) arr_get(handle, j) == str[count] && count != len) {
            count++;
        }

        if (count == len)
            return j;
        else count = 0;
    }
    return -1;
}

API string_handle str_path_ext(string_handle handle) {
    int32_t sepid = str_find_last(handle, ".");
    if (sepid != -1)
        return str_substring(handle, sepid + 1, str_get_size(handle) - sepid - 1);
    return 0;
}

API string_handle str_path_filename(string_handle handle)
{
    int32_t sepid = str_find_last(handle, "/");
    int32_t size = str_get_size(handle);
    if(sepid != -1)
        return str_substring(handle, sepid+1,  size - sepid-1);
    return 0;
}

string_handle str_path_dir(string_handle handle) {

    int32_t sepid = str_find_last(handle, "/");
    if(sepid != -1)
        return str_substring(handle, 0, sepid);
    return 0;
}

void str_delete(string_handle handle) {
    arr_delete(handle);
}