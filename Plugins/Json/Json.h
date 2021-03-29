/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_JSON_H
#define FIXEDPHYSICS_JSON_H

#include "Common.h"

typedef enum {
    JSN_UNDEFINED = 0,
    JSN_OBJECT = 1,
    JSN_ARRAY = 2,
    JSN_STRING = 3,
    JSN_PRIMITIVE = 4
} jsntype_t;

enum jsnerr {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct jsntok {
    jsntype_t type;
    int start;
    int end;
    int size;
#ifdef JSN_PARENT_LINKS
    int parent;
#endif
} jsntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct jsn_parser {
    unsigned int pos;     /* offset in the JSON string */
    unsigned int toknext; /* next token to allocate */
    int toksuper;         /* superior token node, e.g. parent object or array */
} jsn_parser;

/**
 * Create JSON parser over an array of tokens
 */
API void jsn_init(jsn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing
 * a single JSON object.
 */
API int jsn_parse(jsn_parser *parser, i8 const*js, u32 len,
                        jsntok_t *tokens, u32 num_tokens);


#endif //FIXEDPHYSICS_JSON_H