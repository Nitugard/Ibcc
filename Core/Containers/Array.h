/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdint.h>

#ifndef OS_ALLOCATOR
#include <stdlib.h>
#define OS_MALLOC(size) malloc(size)
#define OS_REALLOC(x, size) realloc(x, size)
#define OS_FREE(x) free(x)
#define OS_ALLOCATOR
#endif

#ifndef API
#define API
#endif

typedef struct arr_data* arr_handle;

/*!
 * Creates new array.
 * @param element_size Space occupied by each element in the array.
 * @param capacity Initial capacity of the array. Can be zero, array is still created.
 * @return Created array handle, otherwise 0.
 */
API arr_handle arr_new(int32_t element_size, int32_t capacity);

/*!
 * Returns array capacity.
 */
API void arr_capacity(arr_handle handle, int32_t capacity);

/*!
 * Updates array size, increasing capacity if necessary.
 */
API void arr_resize(arr_handle handle, int32_t length);

/*!
 * Updates array capacity to match current array size.
 */
API void arr_trim(arr_handle handle);

/*!
 * Returns whether the array is empty.
 */
API bool arr_empty(arr_handle handle);

/*!
 * Returns whether the array is full.
 */
API bool arr_full(arr_handle handle);

/*!
 * Adds a new element in the array, increasing its size and capacity(if necessary).
 * Element memory to which it is pointing is copied in the array where copy length is
 * already defined element size.
 */
API void arr_add(arr_handle handle, void* element);

/*!
 * Adds a number of elements in the array.
 */
API void arr_add_range(arr_handle handle, void** element, int32_t length);

/*!
 * Removes element at index from the array.
 * Index is zero based.
 * Index must be less than array size.
 */
API void arr_remove(arr_handle handle, int32_t index);

/*!
 * Removes a number of elements from the array.
 * Index is zero based.
 * Index must be less than array size.
 */
API void arr_remove_range(arr_handle handle, int32_t index, int32_t length);

/*!
 * Removes last element from the array.
 */
API void arr_remove_last(arr_handle handle);

/*!
 * Removes element O(1) from the array, order is not preserved.
 * Index is zero based.
 * Index must be less or equal to array size.
 */
API void arr_remove_swap(arr_handle handle, int32_t index);

/*!
 * Inserts element before index. Index is zero based.
 * Index must be less or equal to array size.
 */
API void arr_insert(arr_handle handle, int32_t index, void* element);

/*!
 * Inserts number of elements before index. Index is zero based.
 * Index must be less or equal to array size.
 */
API void arr_insert_range(arr_handle handle, int32_t index, void** element, int32_t length);

/*!
 * Returns number of elements present in array.
 */
API int32_t arr_size(arr_handle handle);

/*!
 * Returns maximum number of elements(capacity) of the array.
 */
API int32_t arr_max_size(arr_handle handle);

/*!
 * Returns element memory from the array at the given index.
 * Index is zero based.
 * Index must be less than array capacity.
 */
API void *arr_get(arr_handle handle, int32_t index);

/*!
 * Overwrites element at given index in the array by overwriting memory.
 * Index is zero based.
 * Index must be less than array capacity.
 */
API void arr_set(arr_handle handle, int32_t index, void* element);

/*!
 * Deletes array and associated data.
 */
API void arr_delete(arr_handle handle);

/*!
 * Returns index of element inside of array whose memory content matches that of provided the element.
 * Returns -1 if such element could not be found.
 * Search is done by comparing 8 bits of memory at the time.
 */
API int32_t arr_find_by_mem_8(arr_handle handle, void* element);

#endif //ARRAY_H

#ifdef ARRAY_IMPLEMENTATION

#include <string.h>

#ifndef CORE_ASSERT
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef struct arr_data
{
    int32_t element_size;
    int32_t cur_count;
    int32_t max_count;

} arr_data;

#define PTR(arr, i) ((void*)((arr) + 1) + (arr)->element_size * (i))

void arr_capacity(arr_handle handle, int32_t capacity) {
    if(capacity == handle->max_count)
        return;
    
    handle = OS_REALLOC(handle, sizeof(arr_data) + handle->element_size * capacity);
    handle->max_count = capacity;

    if(handle->cur_count > handle->max_count)
        handle->cur_count = handle->max_count;

    CORE_ASSERT(handle != 0 && "Could not reallocate array");
}

arr_handle arr_new(int32_t element_size, int32_t capacity) {
    arr_handle arr = OS_MALLOC(sizeof(arr_data) + (element_size * capacity));
    arr->element_size = element_size;
    arr->cur_count = 0;
    arr->max_count = capacity;
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
        memcpy(PTR(arr, arr->cur_count), PTR(arr, arr->cur_count + 1), arr->element_size);
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

void arr_set(arr_handle arr, int32_t index, void * e) {
    CORE_ASSERT(index >= 0 && index < arr->cur_count);
    memcpy(PTR(arr, index), e, arr->element_size);
}

void arr_delete(arr_handle handle) {
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
    if(length > handle->max_count)
        arr_capacity(handle, length);
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
#undef ARRAY_IMPLEMENTATION
#endif