/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Allocator.h"
#include "File.h"

#include <stdio.h> //fopen fread fclose
#ifdef __MINGW32__
#include <unistd.h> //access
#endif

#include "assert.h"
#define CORE_ASSERT(e) assert(e)


#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#include <fcntl.h>

#endif

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

#if __MINGW32__
    if (access(name, F_OK) == 0) {
        return true;
    }
    else{
        return false;
    }
#else
    FILE* file = fopen(name, "r");
    bool result = file != 0;
    if(result) fclose(file);
    return result;
#endif
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

unsigned char *file_mmap(const char *path, uint32_t* length) {
    *length = 0;
#ifdef _WIN32
    HANDLE file =
            CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return NULL;

    HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    CORE_ASSERT(fileMapping != INVALID_HANDLE_VALUE);

    LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
    unsigned char *fileMapViewChar = (unsigned char *) fileMapView;
    CORE_ASSERT(fileMapView != NULL);

    DWORD file_size = GetFileSize(file, NULL);
    (*length) = (size_t) file_size;

    return fileMapViewChar;

#else
#error "mmap not implemented"
#endif
}
