/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FILE_H
#define FILE_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif


typedef struct file_buffer{
    void* memory;
    int32_t size;
} file_buffer;

typedef struct file_data* file_hndl;

API const char* file_path(file_hndl hndl);

API void* file_get_hndl(file_hndl);
API file_hndl file_open(char const*, const char*);
API bool file_exists(char const*);
API int32_t file_size(file_hndl);

API void file_write(file_hndl, file_buffer const*);
API void file_read(file_hndl, file_buffer *);

API void file_close(file_hndl);

API unsigned char* file_mmap(char const* path, uint32_t* length);

#endif //FILE_H
