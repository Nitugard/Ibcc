/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_VFS_H
#define FIXEDPHYSICS_VFS_H

#include <Common.h>

typedef struct file* file_hndl;

typedef struct vfs_start_desc{
    const char* folder;
}vfs_start_desc;


API void vfs_start(vfs_start_desc const*);
API void vfs_fload();
API void vfs_funload();
API void vfs_stop();

#endif //FIXEDPHYSICS_VFS_H
