/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <ThirdParty/jsmn/jsmn.h>

#include <Os/Allocator.h>
#include <Os/File.h>
#include <Asset/Asset.h>
#include <Log/Log.h>
#include <string.h>
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

void asset_on_unload_json(asset_hndl hndl)
{
    json_hndl jhndl = hndl;
    OS_FREE(jhndl->tokens);
    OS_FREE(jhndl);
}

i32 json_token_find(json_hndl hndl, i8 const* name, json_token_type type, i32 offset, i32 length) {
    i32 len = strlen(name);
    for(i32 i=offset; i<length; ++i)
    {
        struct json_token t = hndl->tokens[i];
        if(json_token_name_cmp(hndl, i, name, len) && t.type == type)
            return i;
    }
    return -1;
}

void json_token_to_str(json_hndl hndl, i32 token, char* buffer) {
    json_token t = hndl->tokens[token];
    OS_MEMCPY(buffer, hndl->blob + t.start, t.end - t.start);
    buffer[t.end - t.start + 1] = '\0';
}

bool json_token_name_cmp(json_hndl hndl, i32 token, const i8 *name, i32 len) {
    struct json_token t = hndl->tokens[token];
    if(t.end - t.start != len)
        return false;

    for(i32 i=t.start; i<t.end; ++i)
    {
        if(hndl->blob[i] != name[i - t.start])
            return false;
    }

    return true;
}

void json_token_print(json_hndl hndl, i32 token) {
    json_token t = hndl->tokens[token];
    char name_buff[1024];
    json_token_to_str (hndl, token, name_buff);
    LOG_INFO("{\nchildren: %i\ntype: %i\nsrc:\n%s\n}\n", t.children, t.type, name_buff);

}

plg_desc req_plugins[] = {};
void plg_on_start(plg_info* info) {

    info->name = "Json";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load(plg_info const* info) {
    asset_init();
    asset_register_desc json_desc = {
            .extension = "json",
            .asset_on_load = asset_on_load_json,
            .asset_on_unload = asset_on_unload_json
    };

    return true;
}

void plg_on_stop(plg_info* info)
{
}
