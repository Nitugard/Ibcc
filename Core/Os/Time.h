/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_TIME_H
#define FIXEDPHYSICS_TIME_H

#include "Common.h"


API i64 time_now();
API bool time_equals(i64, i64);

API i64 time_file_created(i8 const*);
API i64 time_file_modified(i8 const*);

#endif //FIXEDPHYSICS_TIME_H
