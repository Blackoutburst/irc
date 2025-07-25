#pragma once

#include <stdlib.h>
#include <string.h>
#include "types.h"

#define MAX_MESSAGE_LEN 512
#define MAX_USERNAME_LEN 16
#define MAX_MESSAGES 100
#define MAX_USERS 10

typedef struct {
    I8 username[MAX_USERNAME_LEN];
    I8 content[MAX_MESSAGE_LEN];
} ChatMessage;

typedef struct {
    I8 input[MAX_MESSAGE_LEN];
    I32 input_len;
    ChatMessage messages[MAX_MESSAGES];
    I32 message_count;
    I8 users[MAX_USERS][MAX_USERNAME_LEN];
    I32 user_count;
    I8 current_channel[256];
    I8 server_name[256];
    I8 server_motd[256];
    I8 username[MAX_USERNAME_LEN];
} ChatState;

void chatInit(void);
ChatState* chatGetState(void);
void chatAddMessage(const I8* username, const I8* message);
