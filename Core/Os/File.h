/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_FILE_H
#define FIXEDPHYSICS_FILE_H

#include <Common.h>


typedef struct file_buffer{
    void* memory;
    i32 size;
} file_buffer;

typedef struct file_data* file_hndl;

API const i8* file_path(file_hndl hndl);

API bool file_path_filename(i8 const* path, i8* out, bool include_ext);
API bool file_path_ext(i8 const* fname, i8* out);
API bool file_path_dir(const i8 *path, i8 *out);

API file_hndl file_open(i8 const*, const i8*);
API bool file_exists(i8 const*);
API i32 file_size(file_hndl);

API void file_write(file_hndl, file_buffer const*);
API void file_read(file_hndl, file_buffer *);

API void file_close(file_hndl);

#endif //FIXEDPHYSICS_FILE_H
