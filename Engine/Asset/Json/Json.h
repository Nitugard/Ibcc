/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_JSON_H
#define FIXEDPHYSICS_JSON_H


#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef enum json_token_type{
    JSON_UNDEFINED = 0,
    JSON_OBJECT = 1,
    JSON_ARRAY = 2,
    JSON_STRING = 3,
    JSON_PRIMITIVE = 4
} json_token_type;

typedef struct json_token{
    json_token_type type;
    int32_t start;
    int32_t end;
    int32_t children;
} json_token;

typedef struct json_data{
    json_token* tokens;
    int32_t tokens_count;
    const char* blob;
} json_data;

typedef json_data* json_hndl;

API bool json_token_name_cmp(json_hndl hndl, int32_t token, char const * name, int32_t len);
API int32_t json_token_find(json_hndl hndl, char const* name, json_token_type type, int32_t offset, int32_t length);
API void json_token_print(json_hndl, int32_t token);
API void json_token_to_str(json_hndl, int32_t token, char* buffer);

#endif //FIXEDPHYSICS_JSON_H