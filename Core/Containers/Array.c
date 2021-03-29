/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "../Os/Allocator.h"

#include "Array.h"

//Todo: resize as multiple of 4
//Todo: range, remove, add

#ifndef ARRAY_ASSERT
#include <assert.h>
#define ARRAY_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef struct arr
{
    i32 element_size;
    i32 cur_count;
    i32 max_count;

} arr_data;

#define PTR(arr, i) ((void*)((arr) + 1) + (arr)->element_size * (i))

void arr_resize(arr_handle arr, int max_count) {
    ARRAY_ASSERT(max_count >= 0);
    arr = OS_REALLOC(arr, arr->element_size * max_count);
    ARRAY_ASSERT(arr != 0 && "Could not reallocate array");
}

arr_handle arr_create(i32 element_size, i32 elements) {
    ARRAY_ASSERT(elements > 0);
    arr_handle arr = OS_MALLOC(sizeof(arr_data) + (element_size * elements));
    arr->cur_count = 0;
    arr->max_count = elements;
    arr->element_size = element_size;
    return arr;
}

void arr_add(arr_handle arr, element_ptr e) {
    if(arr->cur_count == arr->max_count)
        arr_resize(arr, arr->cur_count + 1);
    ARRAY_ASSERT(arr->cur_count < arr->max_count);
    OS_MEMCPY(PTR(arr, arr->cur_count), e, arr->element_size);
    arr->cur_count++;
}

void arr_remove(arr_handle arr, i32 index) {
    ARRAY_ASSERT(index >= 0 && index < arr->cur_count);
    for(i32 i=index; i<arr->cur_count-1; ++i) {
        OS_MEMCPY(PTR(arr, arr->cur_count), PTR(arr, arr->cur_count + 1), arr->element_size);
    }
    --arr->cur_count;
}

i32 arr_size(arr_handle arr) {
    return arr->cur_count;
}

i32 arr_search(arr_handle arr, element_ptr e) {
    for(i32 i=0; i<arr->cur_count; ++i) {
        if(OS_MEMCPY(PTR(arr, i), e, arr->element_size) == 0)
            return i;
    }

    return -1;
}

element_ptr arr_get(arr_handle arr, i32 index) {
    ARRAY_ASSERT(index >= 0 && index < arr->cur_count);
    return PTR(arr, index);
}

void arr_set(arr_handle arr, i32 index, element_ptr e) {
    ARRAY_ASSERT(index >= 0 && index < arr->cur_count);
    OS_MEMCPY(PTR(arr, arr->cur_count), e, arr->element_size);
}

void arr_destroy(arr_handle arr) {
    OS_FREE(arr);
}
