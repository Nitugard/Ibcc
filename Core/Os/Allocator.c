/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>

#include "Allocator.h"
#include "../Log/Log.h"

typedef struct os_proxy_header{
    const i8* file;
    i32 line;
    i32 size;
    bool realloc;
    bool valid;
    i32 index;
} os_proxy_header;

i32 total_allocations;
i32 total_size;

#define ALLOC(x) malloc(x)
#define REALLOC(x, size) realloc(x, size)
#define FREE(x) free(x)

#define TRACKED_ALLOCATIONS (1024 * 128)
os_proxy_header* allocations;

void* os_allocate_proxy(i32 size, const i8 * file, i32 line) {
    os_proxy_header* mem = ALLOC(size + sizeof(os_proxy_header));
    mem->file = file;
    mem->line = line;
    mem->size = size;
    mem->realloc = false;
    mem->valid = true;
    total_allocations++;
    total_size += size;
    for(i32 i=0; i<TRACKED_ALLOCATIONS; ++i)
    {
        if(!allocations[i].valid)
        {
            mem->index = i;
            allocations[i] = *mem;
            break;
        }
    }
    return (void*)(mem + 1);
}

void* os_reallocate_proxy(void* hndl, i32 new_size, const i8 * file, i32 line) {
    os_proxy_header* mem = (os_proxy_header*)(hndl) - 1;
    total_allocations--;
    total_size -= mem->size;
    mem = REALLOC(mem, new_size + sizeof(os_proxy_header));
    //todo: assert null
    mem->line = line;
    mem->file = file;
    mem->size = new_size;
    mem->realloc = true;
    mem->valid = true;
    allocations[mem->index] = *mem;
    total_allocations++;
    total_size += new_size;
    return (void*)(mem + 1);
}

void os_free_proxy(void* hndl, const i8 * file, i32 line) {
    os_proxy_header* mem = (os_proxy_header*)(hndl) - 1;
    total_allocations--;
    total_size -= mem->size;
    mem->valid = false;
    allocations[mem->index] = *mem;
    FREE(hndl);
}

void* os_memcpy(void* dest, void const* src, i32 size) {
    return memcpy(dest, src, size);
}

void* os_memset(void* dest, i32 value, i32 size) {
    return memset(dest, value, size);
}

bool os_assert_memory() {
    if(total_allocations > 0)
        return false;
    if(total_size > 0)
        return false;

    return true;
}
void os_allocator_init() {
    i32 alloc_size = TRACKED_ALLOCATIONS * sizeof(os_proxy_header);
    allocations = ALLOC(alloc_size);
    os_memset(allocations, 0, alloc_size);
}

void os_allocator_finalize() {
    if(!os_assert_memory())
    {
        LOG_FATAL("Memory leak, num allocations: %i, memory: %i\n", total_allocations, total_size);
    }
    for(i32 i=0; i<TRACKED_ALLOCATIONS; ++i) {
        struct os_proxy_header header = allocations[i];
        if (header.valid) {
            LOG_FATAL("Memory leak, size: %i, realloc: %i, src:\n%s:%i\n", header.size,
                      header.realloc, header.file, header.line);
        }
    }

    FREE(allocations);
}


