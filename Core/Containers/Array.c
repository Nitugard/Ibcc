/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Array.h"
#include <string.h>


#include "assert.h"
#define CORE_ASSERT(e) assert(e)

typedef struct arr_data
{
    void* data;
    int32_t element_size;
    int32_t cur_count;
    int32_t max_count;

} arr_data;

#define PTR(arr, i) ((char*)((arr)->data) + (arr)->element_size * (i))

void arr_capacity(arr_handle handle, int32_t capacity) {
    if(capacity == handle->max_count)
        return;

    handle->data = OS_REALLOC(handle->data, handle->element_size * capacity);
    handle->max_count = capacity;

    if(handle->cur_count > handle->max_count)
        handle->cur_count = handle->max_count;
    else
        memset(arr_get(handle, handle->cur_count), 0, handle->element_size * (handle->max_count - handle->cur_count));

    CORE_ASSERT(handle != 0 && "Could not reallocate array");
}

arr_handle arr_new(int32_t element_size, int32_t capacity) {
    arr_handle arr = OS_MALLOC(sizeof(arr_data));
    arr->data = OS_MALLOC(element_size * capacity);
    arr->element_size = element_size;
    arr->cur_count = 0;
    arr->max_count = capacity;
    memset(arr->data, 0, element_size * capacity);
    return arr;
}

void arr_add(arr_handle arr, void * e) {
    if(arr->cur_count == arr->max_count)
        arr_capacity(arr, arr->cur_count + 1);
    memcpy(PTR(arr, arr->cur_count), e, arr->element_size);
    arr->cur_count++;
}

void arr_remove(arr_handle arr, int32_t index) {
    CORE_ASSERT(index >= 0 && index < arr->cur_count);
    for(int32_t i=index; i < arr->cur_count - 1; ++i) {
        memcpy(PTR(arr, i), PTR(arr, i + 1), arr->element_size);
    }
    --arr->cur_count;
}

int32_t arr_size(arr_handle handle) {
    return handle->cur_count;
}

void * arr_get(arr_handle arr, int32_t index) {
    CORE_ASSERT(index >= 0 && index < arr->max_count);
    return PTR(arr, index);
}

API void* arr_get_last(arr_handle handle) {
    if (arr_empty(handle)) return 0;
    return PTR(handle, arr_size(handle) - 1);
}

void arr_set(arr_handle arr, int32_t index, void * e) {
    CORE_ASSERT(index >= 0 && index < arr->cur_count);
    memcpy(PTR(arr, index), e, arr->element_size);
}

void arr_delete(arr_handle handle) {
    OS_FREE(handle->data);
    OS_FREE(handle);
}

int32_t arr_find_by_mem_8(arr_handle handle, void * element) {
    const char* t = element;
    for(int32_t i=0; i < arr_size(handle); ++i)
    {
        const char* e = (const char*)arr_get(handle, i);
        bool found = true;
        for(int32_t j=0; j<handle->element_size; ++j)
        {
            if(t[j] != e[j]){
                found = false;
                break;
            }
        }
        if(found) return i;
    }
    return -1;
}

void arr_trim(arr_handle handle) {
    if(handle->cur_count < handle->max_count)
        arr_capacity(handle, handle->cur_count);
}

int32_t arr_max_size(arr_handle handle) {
    return handle->max_count;
}

void *arr_internal(arr_handle handle, int32_t offset) {
    CORE_ASSERT(offset < handle->max_count);
    return PTR(handle, offset);
}

void arr_add_range(arr_handle handle, void * *element, int32_t length) {
    arr_insert_range(handle, handle->cur_count, element, length);
}

void arr_resize(arr_handle handle, int32_t length) {
    if(length > handle->max_count){
        arr_capacity(handle, length);
    }
    handle->cur_count = length;
}

void arr_remove_range(arr_handle handle, int32_t index, int32_t length) {
    CORE_ASSERT(handle->cur_count > 0);
    CORE_ASSERT(index +length < handle->cur_count);
    for (int32_t i = index; i < handle->cur_count - length; ++i)
        memcpy(PTR(handle, i), PTR(handle, i + length), handle->element_size);
    handle->cur_count -= length;
}

void arr_insert(arr_handle handle, int32_t index,  void *element) {
    arr_insert_range(handle, index, &element, 1);
}

void arr_insert_range(arr_handle handle, int32_t index,  void **element, int32_t length) {
    CORE_ASSERT(index <= handle->cur_count || index == handle->cur_count == 0);
    if(handle->cur_count + length > handle->max_count)
        arr_capacity(handle, handle->cur_count + length);

    for(int32_t i= handle->cur_count + length - 1; i >= index + length; --i)
        memcpy(PTR(handle, i),  PTR(handle, (i - length)), handle->element_size);

    for(int32_t i=index; i< index + length; ++i)
        memcpy(PTR(handle, i), element[i - index], handle->element_size);

    handle->cur_count += length;
}

void arr_remove_last(arr_handle handle) {
    CORE_ASSERT(handle->cur_count != 0);
    handle->cur_count--;
}

void arr_remove_swap(arr_handle handle, int32_t index) {
    CORE_ASSERT(handle->cur_count != 0);
    if (handle->cur_count > 1) {
        memcpy(PTR(handle, index), PTR(handle, handle->cur_count - 1), handle->element_size);
    }
    arr_remove_last(handle);
}

bool arr_empty(arr_handle handle) {
    return handle->cur_count == 0;
}

bool arr_full(arr_handle handle) {
    return handle->cur_count == handle->max_count;
}
