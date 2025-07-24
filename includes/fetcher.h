#pragma once

#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

typedef struct httpRes HTTP_RES;

struct httpRes {
    I8* content;
    ANY size;
};

I32 fetcherInit(void);
void fetcherClean(void);
I8* fetcherPost(const I8* url, const I8* body);
