/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ALLOCATOR_H
#define FIXEDPHYSICS_ALLOCATOR_H

//todo: simple allocator that is just a prxoy to malloc

#define DEBUG_ALLOCATOR

#include "Common.h"


#define OS_MALLOC(size) os_allocate_proxy(size, __FILE__, __LINE__)
#define OS_REALLOC(ptr, new_size) os_reallocate_proxy(ptr, new_size, __FILE__, __LINE__)
#define OS_FREE(ptr) os_free_proxy(ptr, __FILE__, __LINE__)
#define OS_MEMCPY(dest, src, size) os_memcpy(dest, src, size)
#define OS_MEMSET(dest, val, size) os_memset(dest, val, size)

API void os_allocator_init();
API void os_allocator_finalize();

API void* os_memcpy(void*, void const*, i32);
API void* os_memset(void*, i32, i32);

API void* os_allocate_proxy(i32, i8 const*, i32);
API void* os_reallocate_proxy(void*, i32, i8 const*, i32);
API void os_free_proxy(void*, i8 const*, i32);

#endif //FIXEDPHYSICS_ALLOCATOR_H
