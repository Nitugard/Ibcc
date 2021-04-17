/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdint.h>


extern void* os_allocate_proxy(uint32_t, char const*, uint32_t);
extern void* os_reallocate_proxy(void*, uint32_t, char const*, uint32_t);
extern void os_free_proxy(void*, char const*, uint32_t);

#define OS_MALLOC(size) os_allocate_proxy(size, __FILE__, __LINE__)
#define OS_REALLOC(ptr, new_size) os_reallocate_proxy(ptr, new_size, __FILE__, __LINE__)
#define OS_FREE(ptr) os_free_proxy(ptr, __FILE__, __LINE__)

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
