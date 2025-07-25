#pragma once

#include <stdlib.h>
#include <string.h>

#include "types.h"

I32 stringCountWord(const I8* string, const I8 delimiter);
I8** stringSplit(const I8* string, const I8 delimiter);
void stringCleanArray(I8** string_array);
void stringClear(I8* string);
