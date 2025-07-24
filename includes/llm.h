#pragma once

#include <stdlib.h>
#include <string.h>
#include "yyjson.h"
#include "fetcher.h"
#include "types.h"

typedef struct llmMessage LLM_MESSAGE;

struct llmMessage {
    I8* role;
    I8* content;
    I32 used;
};

void llmInit(const I8* model, const I32 context_size, const I8* system_prompt);
void llmClean(void);
I8* llmSpeak(const I8* msg);

void _llmAddMessage(const I8* role, const I8* content);
I8* _llmFormatInput(void);
void _llmFillMessage(LLM_MESSAGE* msg, const I8* role, const I8* content);
I8* _llmGetAnswer(I8* data);
void _llmFreeMessage(LLM_MESSAGE* msg);

