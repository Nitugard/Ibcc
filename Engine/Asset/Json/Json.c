/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <ThirdParty/jsmn/jsmn.h>

#include <Os/Allocator.h>
#include <Os/File.h>
#include <Asset/Asset.h>
#include <Os/Log.h>

#include <string.h>
#include "Json.h"



asset_hndl asset_on_load_json(char* path) {
    file_hndl hndl = file_open(path, "r");
    int32_t size = file_size(hndl);
    json_hndl json_hndl = OS_MALLOC(sizeof(struct json_data) + sizeof(char) * size);
    file_buffer buffer = {.size = size, .memory = json_hndl + 1};
    json_hndl->blob = buffer.memory;
    int32_t maximum_tokens = 1024;
    json_hndl->tokens = OS_MALLOC(maximum_tokens * sizeof(struct json_token));
    json_hndl->tokens_count = maximum_tokens;
    file_read(hndl, &buffer);
    jsmn_parser parser;
    jsmn_init(&parser);
    int32_t err = 0;
    do{
        if(err == JSMN_ERROR_NOMEM)
        {
            json_hndl->tokens = OS_REALLOC(json_hndl->tokens,
                                           json_hndl->tokens_count * 2 * sizeof(struct json_token));
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
    file_close(hndl);
    return (asset_hndl) json_hndl;
}

void asset_on_unload_json(asset_hndl hndl)
{
    json_hndl jhndl = hndl;
    OS_FREE(jhndl->tokens);
    OS_FREE(jhndl);
}

int32_t json_token_find(json_hndl hndl, char const* name, json_token_type type, int32_t offset, int32_t length) {
    int32_t len = strlen(name);
    for(int32_t i=offset; i < length; ++i)
    {
        struct json_token t = hndl->tokens[i];
        if(json_token_name_cmp(hndl, i, name, len) && t.type == type)
            return i;
    }
    return -1;
}

void json_token_to_str(json_hndl hndl, int32_t token, char* buffer) {
    json_token t = hndl->tokens[token];
    os_memcpy(buffer, hndl->blob + t.start, t.end - t.start);
    buffer[t.end - t.start + 1] = '\0';
}

bool json_token_name_cmp(json_hndl hndl, int32_t token, const char *name, int32_t len) {
    struct json_token t = hndl->tokens[token];
    if(t.end - t.start != len)
        return false;

    for(int32_t i=t.start; i < t.end; ++i)
    {
        if(hndl->blob[i] != name[i - t.start])
            return false;
    }

    return true;
}

void json_token_print(json_hndl hndl, int32_t token) {
    json_token t = hndl->tokens[token];
    char name_buff[1024];
    json_token_to_str (hndl, token, name_buff);
    LOG_INFO("{\nchildren: %i\ntype: %i\nsrc:\n%s\n}\n", t.children, t.type, name_buff);

}


void init_json_asset() {
    asset_register_desc json_desc = {
            .extension = "json",
            .asset_on_load = asset_on_load_json,
            .asset_on_unload = asset_on_unload_json,
    };
    asset_register(&json_desc);
}
