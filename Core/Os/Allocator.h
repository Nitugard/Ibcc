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

#define OS_MALLOC(size) os_allocate_proxy(size, __FILE__, __LINE__)
#define OS_REALLOC(ptr, new_size) os_reallocate_proxy(ptr, new_size, __FILE__, __LINE__)
#define OS_FREE(ptr) os_free_proxy(ptr, __FILE__, __LINE__)

API void os_allocator_init();
API void os_allocator_finalize();

API void os_memcpy(void *src, void const *dest, int32_t size);
API void os_memset(void *src, int32_t value, int32_t size);

API void *os_allocate_proxy(int32_t size, char const *file, int32_t line);
API void *os_reallocate_proxy(void *src, int32_t size, char const *file, int32_t line);
API void os_free_proxy(void *src, char const *file, int32_t line);

API int32_t os_get_tracked_allocations_length();
API void os_get_tracked_allocations(os_proxy_header const** allocations);


#endif //ALLOCATOR_H

#ifdef ALLOCATOR_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#ifndef CORE_ASSERT
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif


#define ALLOC(x) malloc(x)
#define REALLOC(x, size) realloc(x, size)
#define FREE(x) free(x)
#define TRACKED_ALLOCATIONS_START_LENGTH 1024
#define TRACKED_ALLOCATIONS_REALLOC_LENGTH 128

int32_t total_allocations;
int32_t total_size;

int32_t tracked_allocations_length;
os_proxy_header** tracked_allocations;

void *os_allocate_proxy(int32_t size, char const *file, int32_t line) {
    os_proxy_header* mem = ALLOC(size + sizeof(os_proxy_header));
    mem->file = file;
    mem->line = line;
    mem->size = size;
    mem->realloc = false;
    total_allocations++;
    total_size += size;
    bool is_tracked = false;
    for(int32_t i=0; i < tracked_allocations_length; ++i)
    {
        if(tracked_allocations[i] == 0)
        {
            mem->index = i;
            tracked_allocations[i] = mem;
            is_tracked = true;
            break;
        }
    }
    if(is_tracked == false) {
        tracked_allocations = realloc(tracked_allocations,
                                      (tracked_allocations_length + TRACKED_ALLOCATIONS_REALLOC_LENGTH) *
                                      sizeof(os_proxy_header *));
        mem->index = tracked_allocations_length;
        tracked_allocations[tracked_allocations_length] = mem;
        tracked_allocations_length += TRACKED_ALLOCATIONS_REALLOC_LENGTH;
    }

    return (void*)(mem + 1);
}

void *os_reallocate_proxy(void *src, int32_t size, char const *file, int32_t line) {
    if(src == 0)
        return os_allocate_proxy(size, file, line);

    os_proxy_header* mem = (os_proxy_header*)(src) - 1;
    CORE_ASSERT(tracked_allocations[mem->index] == mem && "Invalid realloc pointer, not allocated by this allocator!");

    total_size -= mem->size;
    mem = REALLOC(mem, size + sizeof(os_proxy_header));
    mem->line = line;
    mem->file = file;
    mem->size = size;
    mem->realloc = true;
    total_size += size;
    return (void*)(mem + 1);
}

void os_free_proxy(void *src, char const *file, int32_t line) {
    os_proxy_header* mem = (os_proxy_header*)(src) - 1;
    total_allocations--;
    total_size -= mem->size;
    CORE_ASSERT(tracked_allocations[mem->index] == mem && "Invalid realloc pointer, not allocated by this allocator!");
    tracked_allocations[mem->index] = 0;
    FREE(src);
}

void os_memcpy(void *src, void const *dest, int32_t size) {
    memcpy(src, dest, size);
}

void os_memset(void *src, int32_t value, int32_t size) {
    memset(src, value, size);
}

bool os_assert_memory() {
    if(total_allocations > 0)
        return false;
    if(total_size > 0)
        return false;

    return true;
}

void os_allocator_init() {
    int32_t size = TRACKED_ALLOCATIONS_START_LENGTH * sizeof(struct os_proxy_header *);
    tracked_allocations = malloc(size);
    os_memset(tracked_allocations, 0, size);
    tracked_allocations_length = TRACKED_ALLOCATIONS_START_LENGTH;
}

int32_t os_get_tracked_allocations_length()
{
    int32_t count = 0;
    struct os_proxy_header* header;
    for(int32_t i=0; i < tracked_allocations_length; ++i) {
        header = tracked_allocations[i];
        if (header != 0) count++;
    }

    return count;
}

void os_get_tracked_allocations(struct os_proxy_header const** allocations)
{
    int32_t count = 0;
    struct os_proxy_header* header;
    for(int32_t i=0; i < tracked_allocations_length; ++i) {
        header = tracked_allocations[i];
        if (header != 0) allocations[count++] = header;
    }
}

void os_allocator_finalize() {
    CORE_ASSERT(os_assert_memory() && "Memory leak");
    free(tracked_allocations);
}

#undef ALLOCATOR_IMPLEMENTATION
#endif