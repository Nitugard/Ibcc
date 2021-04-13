/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_JSON_H
#define FIXEDPHYSICS_JSON_H

#include "Common.h"

typedef enum json_token_type{
    JSON_UNDEFINED = 0,
    JSON_OBJECT = 1,
    JSON_ARRAY = 2,
    JSON_STRING = 3,
    JSON_PRIMITIVE = 4
} json_token_type;

typedef struct json_token{
    json_token_type type;
    i32 start;
    i32 end;
    i32 children;
} json_token;

typedef struct json_data{
    json_token* tokens;
    i32 tokens_count;
    const i8* blob;
} json_data;

typedef json_data* json_hndl;

API bool json_token_name_cmp(json_hndl hndl, i32 token, i8 const * name, i32 len);
API i32 json_token_find(json_hndl hndl, i8 const* name, json_token_type type, i32 offset, i32 length);
API void json_token_print(json_hndl, i32 token);
API void json_token_to_str(json_hndl, i32 token, char* buffer);

#endif //FIXEDPHYSICS_JSON_H