/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Shader.h"
#include <Asset/Asset.h>
#include <Os/Allocator.h>
#include <Os/File.h>
#include <Asset/Json/Json.h>
#include <Log/Log.h>
#include <string.h>
#define SHADER_TYPE "glsl"

void set_shader_name(const i8* fname, const i8* stage_type, i8* buff){
    OS_MEMSET(buff, 0, sizeof(buff));
    strcat(buff, fname);
    strcat(buff, "_");
    strcat(buff, stage_type);
    strcat(buff, SHADER_TYPE);
}

bool load_frag_shader(const i8* fname, shader_t* shader)
{
    char temp_buffer[1024];
    set_shader_name(fname, "fs", temp_buffer);
    if(!file_exists(temp_buffer))
        return false;

    return true;
}

bool load_vert_shader(const i8* fname, shader_t* shader)
{
    char temp_buffer[1024];
    set_shader_name(fname, "vs", temp_buffer);
    if(!file_exists(temp_buffer))
        return false;

    return true;
}

asset_hndl asset_on_load_shader(void* hptr) {
    file_hndl hndl = (file_hndl) hptr;
    char dir_buffer[1024];
    char filename_buffer[1024];

    OS_MEMSET(dir_buffer, 0, sizeof(dir_buffer));
    OS_MEMSET(filename_buffer, 0, sizeof(filename_buffer));

    const i8 *fp = file_path(hndl);

    file_path_dir(fp, dir_buffer);
    file_path_filename(fp, filename_buffer, false);

    strcat(dir_buffer, SHADER_TYPE);
    strcat(dir_buffer, "/");

    shader_t shader = {.name = filename_buffer, .attributes_count = 0, .uniforms_count = 0};

    if(!load_frag_shader(filename_buffer, &shader))
        return 0;
    if(!load_vert_shader(filename_buffer, &shader))
        return 0;

    LOG_INFO("Shader loaded %s", filename_buffer);

    shader_hndl shndl = OS_MALLOC(sizeof(struct shader_t));
    OS_MEMCPY(shndl, &shader, sizeof(shader_t));
    return (asset_hndl)shndl;
}

