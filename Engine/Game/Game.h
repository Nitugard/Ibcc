/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

API bool game_init();
API void game_terminate();

#endif