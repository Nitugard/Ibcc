/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Allocator.h"
#include <stdlib.h>
#include <string.h>


#ifndef CORE_ASSERT
#ifdef __MINGW32__
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#else
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif
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

typedef struct os_chunk{
    void* ptr;
    uint32_t cur_size;
    uint32_t max_size;
} os_chunk;

void *os_allocate_proxy(uint32_t size, char const *file, uint32_t line) {
    os_proxy_header* mem = ALLOC(size + sizeof(os_proxy_header));
    CORE_ASSERT(mem != 0);
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

void *os_reallocate_proxy(void *src, uint32_t size, char const *file, uint32_t line) {
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

void os_free_proxy(void *src, char const *file, uint32_t line) {
    os_proxy_header* mem = (os_proxy_header*)(src) - 1;
    total_allocations--;
    total_size -= mem->size;
    CORE_ASSERT(tracked_allocations[mem->index] == mem && "Invalid realloc pointer, not allocated by this allocator!");
    tracked_allocations[mem->index] = 0;
    FREE(mem);
}

char* os_memcpy(void *dest, void const *src, int32_t size) {
    return memcpy(dest, src, size);
}

char* os_memset(void *src, int32_t value, int32_t size) {
    return memset(src, value, size);
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

os_chunk_handle os_chunk_new(int32_t initial_capacity) {
    os_chunk_handle handle = OS_MALLOC(initial_capacity + sizeof(struct os_chunk));
    handle->ptr = handle + 1;
    handle->max_size = initial_capacity;
    handle->cur_size = 0;
    return handle;
}

bool os_chunk_alloc(os_chunk_handle handle, uint32_t size, void **pptr) {
    if (handle->cur_size + size > handle->max_size) return false;
    *pptr = handle->ptr + handle->cur_size;
    handle->cur_size += size;
    return true;
}

void os_chunk_free(os_chunk_handle handle) {
    OS_FREE(handle);
}

