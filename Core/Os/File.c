/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include "File.h"
#include "Allocator.h"
#include "unistd.h"

//TODO: BUFFER OVERFLOW CHECH

typedef struct file_data{
    FILE* hndl;
    const i8* fpath;

} file_data;

file_hndl file_open(const i8 * name, const i8* mode) {
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

bool file_exists(const i8 * name) {
    if (access(name, F_OK) == 0) {
        return true;
    }
    else{
        return false;
    }
}

i32 file_size(file_hndl hndl) {
    fseek(hndl->hndl, 0, SEEK_END);
    i32 size = ftell(hndl->hndl);
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

i32 str_last(const i8* s, const i8 t)
{
    i32 id = -1;
    i32 count = 0;
    while(*(s + count) != '\0')
    {
        if(*(s + count) == t)
            id = count;
        ++count;
    }

    return id;
}

bool file_path_ext(const i8 *filename, i8 *out) {
    i32 sepid = str_last(filename, '.');
    if(sepid != -1)
    {
        const char* temp_path = filename + sepid + 1;
        i32 len = strlen(temp_path);
        OS_MEMCPY(out, temp_path, len+1);
        return true;
    }
    return false;
}

bool file_path_filename(const i8 *path, i8 *out, bool include_ext) {
    i32 sepid = str_last(path, '/');
    if(sepid != -1)
    {
        const char* temp_path = path + sepid + 1;
        i32 flen = strlen(temp_path);
        i32 dotid = str_last(temp_path, '.');

        if(flen > 1 && dotid >= 0) {
            if(!include_ext)
            {
                OS_MEMCPY(out, temp_path, dotid);
                *(out + dotid) = '\0';
            }
            else {
                OS_MEMCPY(out, temp_path, flen + 1);
            }
            return true;
        }
        return false;
    }

    return false;
}

const i8 *file_path(file_hndl hndl) {
    return hndl->fpath;
}

bool file_path_dir(const i8 *path, i8 *out) {
    i32 sepid = str_last(path, '/');

    if(sepid != -1)
    {
        OS_MEMCPY(out, path, sepid);
        *(out + sepid + 1) = '\0';
        return true;
    }

    return false;
}
