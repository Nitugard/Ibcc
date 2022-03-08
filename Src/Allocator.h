/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_ALLOCATOR_H
#define IBCWEB_ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>

#ifndef IBC_API
#define IBC_API extern
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

IBC_API void os_allocator_init();
IBC_API void os_allocator_terminate();

IBC_API os_chunk_handle os_chunk_new(int32_t initial_capacity);
IBC_API bool os_chunk_alloc(os_chunk_handle handle, uint32_t size, void** pptr);
IBC_API void os_chunk_free(os_chunk_handle handle);

IBC_API char* os_memcpy(void *dest, void const *src, int32_t size);
IBC_API char* os_memset(void *src, int32_t value, int32_t size);

IBC_API void *os_allocate_proxy(uint32_t size, char const *file, uint32_t line);
IBC_API void *os_reallocate_proxy(void *src, uint32_t size, char const *file, uint32_t line);
IBC_API void os_free_proxy(void *src, char const *file, uint32_t line);

IBC_API int32_t os_get_tracked_allocations_length();
IBC_API int32_t os_get_tracked_allocations_size();
IBC_API void os_get_tracked_allocations(os_proxy_header const** allocations);

#endif //IBCWEB_ALLOCATOR_H

