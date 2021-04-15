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

#endif //FILE_H

#ifdef FILE_IMPLEMENTATION

#include <stdio.h> //fopen fread fclose
#include <unistd.h> //access


typedef struct file_data{
    FILE* hndl;
    const char* fpath;

} file_data;

file_hndl file_open(const char * name, const char* mode) {
    FILE* file = fopen(name, mode);
    if(file != 0)
    {
        file_hndl data = OS_MALLOC(sizeof(struct file_data));
        data->fpath = name;
        data->hndl = file;

        return data;
    }
    return 0;
}

bool file_exists(const char * name) {
    if (access(name, F_OK) == 0) {
        return true;
    }
    else{
        return false;
    }
}

int32_t file_size(file_hndl hndl) {
    fseek(hndl->hndl, 0, SEEK_END);
    int32_t size = ftell(hndl->hndl);
    fseek(hndl->hndl, 0, SEEK_SET);
    return size;
}

void file_write(file_hndl hndl, const file_buffer * buffer) {
    fwrite(buffer->memory, buffer->size, 1, hndl->hndl);
}

void file_read(file_hndl hndl, file_buffer * buffer) {
    fread((char*)buffer->memory, buffer->size, 1, hndl->hndl);
}

void file_close(file_hndl hndl) {
    fclose(hndl->hndl);
    OS_FREE(hndl);
}

void *file_get_hndl(file_hndl hndl) {
    return hndl->hndl;
}

const char *file_path(file_hndl hndl) {
    return hndl->fpath;
}
#undef FILE_IMPLEMENTATION
#endif