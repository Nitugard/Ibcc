/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <ThirdParty/Jsmn/jsmn.h>

#include <Os/Allocator.h>
#include <Os/File.h>
#include <Asset/Asset.h>
#include <Log/Log.h>
#include <Plugin/Plugin.h>

#include "Json.h"

asset_hndl asset_on_load_json(void* hptr) {
    file_hndl hndl = (file_hndl)hptr;
    i32 size = file_size(hndl);
    json_hndl json_hndl = OS_MALLOC(sizeof(struct json_data) + sizeof(i8) * size);
    file_buffer buffer = {.size = size, .memory = json_hndl + 1};
    json_hndl->blob = buffer.memory;
    i32 maximum_tokens = 1024;
    json_hndl->tokens = OS_MALLOC(maximum_tokens * sizeof(struct json_token));
    json_hndl->tokens_count = maximum_tokens;
    file_read(hndl, &buffer);
    jsmn_parser parser;
    jsmn_init(&parser);
    i32 err = 0;
    do{
        if(err == JSMN_ERROR_NOMEM)
        {
            json_hndl->tokens = OS_REALLOC(json_hndl->tokens, json_hndl->tokens_count * 2 * sizeof(struct json_token));
            json_hndl->tokens_count = json_hndl->tokens_count * 2;
        }
        err = jsmn_parse(&parser, buffer.memory, buffer.size, (jsmntok_t *) json_hndl->tokens,
                   json_hndl->tokens_count);

        if(err == JSMN_ERROR_INVAL)
        {
            LOG_ERROR("Error while parsing json, invalid token encountered");
            break;
        }
        if(err == JSMN_ERROR_PART)
        {
            LOG_ERROR("Error while parsing json, expected more data");
            break;
        }
    }
    while(err == JSMN_ERROR_NOMEM);
    json_hndl->tokens_count = err;
    return (asset_hndl) json_hndl;
}
