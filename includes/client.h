#pragma once

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "irc.h"

#define BUFFER_SIZE 1024

I32 clientConnect(const I8* ip, U16 port);
void* clientRead(void* arg);
void clientClose(void);
I32 clientGetfd(void);
pthread_t clientGetThread(void);
