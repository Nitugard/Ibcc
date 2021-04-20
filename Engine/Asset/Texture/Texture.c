/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "Texture.h"

#include <Os/Allocator.h>
#include <Asset/Asset.h>
#include <Os/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(size) OS_MALLOC(size)
#define STBI_REALLOC(x, size) OS_REALLOC(x, size)
#define STBI_FREE(x) OS_FREE(x)
#include <ThirdParty/stb_image/stb_image.h>

asset_hndl asset_on_load_tex(char* path) {
    tex_data *data = OS_MALLOC(sizeof(tex_data));
    data->data = stbi_load(path, &(data->width), &(data->height), &(data->num_channels), 0);
    if(data->data == 0) {
        LOG_ERROR("Texture load failed\n");
    }

    return (asset_hndl) data;
}

void asset_on_unload_tex(asset_hndl hndl)
{
    stbi_image_free(((struct tex_data*)hndl)->data);
    OS_FREE(hndl);
}

void init_texture_asset() {
    asset_register_desc png_desc = {
            .extension = "png",
            .asset_on_load = asset_on_load_tex,
            .asset_on_unload = asset_on_unload_tex
    };
    asset_register(&png_desc);
}
