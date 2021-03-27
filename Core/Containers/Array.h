/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ARRAY_H
#define FIXEDPHYSICS_ARRAY_H


#include "../Types.h"

#define ARRAY_DEBUG

#ifndef ARRAY_ASSERT
#include <assert.h>
#define ARRAY_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

#ifndef API
#define API extern
#endif

typedef struct arr* arr_handle;
typedef void* element_ptr;

API arr_handle arr_create(i32, i32);
API void arr_add(arr_handle, element_ptr);
API void arr_remove(arr_handle, i32);
API i32 arr_size(arr_handle);
API i32 arr_search(arr_handle, element_ptr);
API element_ptr arr_get(arr_handle, i32);
API void arr_set(arr_handle, i32, element_ptr);
API void arr_destroy(arr_handle);


#endif //FIXEDPHYSICS_ARRAY_H
