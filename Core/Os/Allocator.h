/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>

#define OS_ALLOCATOR

#ifndef API
#define API
#endif

typedef struct os_proxy_header{
    const char* file;
    int32_t line;
    int32_t size;
    bool realloc;
    int32_t index;
} os_proxy_header;

typedef struct os_chunk* os_chunk_handle;

#define OS_MALLOC(size) os_allocate_proxy(size, __FILE__, __LINE__)
#define OS_REALLOC(ptr, new_size) os_reallocate_proxy(ptr, new_size, __FILE__, __LINE__)
#define OS_FREE(ptr) os_free_proxy(ptr, __FILE__, __LINE__)

API void os_allocator_init();
API void os_allocator_terminate();

API os_chunk_handle os_chunk_new(int32_t initial_capacity);
API bool os_chunk_alloc(os_chunk_handle handle, uint32_t size, void** pptr);
API void os_chunk_free(os_chunk_handle handle);

API char* os_memcpy(void *dest, void const *src, int32_t size);
API char* os_memset(void *src, int32_t value, int32_t size);

API void *os_allocate_proxy(uint32_t size, char const *file, uint32_t line);
API void *os_reallocate_proxy(void *src, uint32_t size, char const *file, uint32_t line);
API void os_free_proxy(void *src, char const *file, uint32_t line);

API int32_t os_get_tracked_allocations_length();
API int32_t os_get_tracked_allocations_size();
API void os_get_tracked_allocations(os_proxy_header const** allocations);

#endif //ALLOCATOR_H
