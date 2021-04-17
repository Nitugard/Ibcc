/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef void* string_handle;

/*!
 * Creates a new string from a c string.
 */
API string_handle str_new(const char * str);

/*!
 * Creates a new string from a string handle. Data is copied.
 */
API string_handle str_new_handle(string_handle str);

/*!
 * Creates a new empty string with initial capacity.
 * Initial capacity must be larger or equal to 1(in order to include null terminated character).
 */
API string_handle str_new_capacity(int32_t capacity);

/*!
 * Concatenates two strings, result is returned and its memory must be freed.
 */
API string_handle str_concat(string_handle base, const char * str);

/*!
 * Concatenates two strings, result is returned and its memory must be freed.
 */
API string_handle str_concat_handle(string_handle base, string_handle str);

/*!
 * Returns substring given by zero index offset and length, resulting memory must be freed.
 */
API string_handle str_substring(string_handle base, int32_t offset, int32_t length);

/*!
 * Returns string size without null terminated character.
 */
API int32_t str_get_size(string_handle handle);

/*!
 * Returns zero based inclusive starting index of first occurring match.
 */
API int32_t str_find(string_handle handle, const char* str);

/*!
 * Returns zero based inclusive starting index of last occurring match.
 */
API int32_t str_find_last(string_handle handle, const char* str);

/*!
 * Returns internal c style string representation.
 */
API const char* str_internal(string_handle handle);

/*!
 * Deletes string and its associated resources.
 */
API void str_delete(string_handle handle);

/*!
 * Returns 0 if equal, 1 if a > b otherwise -1.
 */
API int32_t str_cmp(const char* a, const char* b);

/*!
 * Returns filename from the path string with extension.
 * Zero otherwise.
 */
API string_handle str_path_filename(string_handle handle);

/*!
 * Returns extension form the path string without dot.
 * Zero otherwise.
 */
API string_handle str_path_ext(string_handle handle);

/*!
 * Returns directory from the path string without last backslash.
 * Zero otherwise.
 */
API string_handle str_path_dir(string_handle handle);

#endif //STRING_H
