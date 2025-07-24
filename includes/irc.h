#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "types.h"

#define IRC_LIST(sockfd) sendPacket(sockfd, "LIST \r\n")
#define IRC_PONG(sockfd, token) sendPacket(sockfd, "PONG %s\r\n", token)
#define IRC_PRIVMSG(sockfd, channel, message) sendPacket(sockfd, "PRIVMSG %s :%s\r\n", channel, message)
#define IRC_PART(sockfd, channel) sendPacket(sockfd, "PART %s\r\n", channel)
#define IRC_JOIN(sockfd, channel) sendPacket(sockfd, "JOIN %s\r\n", channel)
#define IRC_NICK(sockfd, nick) sendPacket(sockfd, "NICK %s\r\n", nick)
#define IRC_USER(sockfd, realname) sendPacket(sockfd, "USER user 0 * :%s\r\n", realname)

void sendPacket(I32 sockfd, const I8* format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);

    I32 required = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (required < 0) {
        perror("vsnprintf failed");
        va_end(args);
        return;
    }

    I32 buffer_size = (size_t)required + 1;
    I8* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc failed");
        va_end(args);
        return;
    }

    I32 written = vsnprintf(buffer, buffer_size, format, args);
    va_end(args);

    if (written < 0 || written >= (I32)buffer_size) {
        perror("vsnprintf failed");
        free(buffer);
        return;
    }

    if (send(sockfd, buffer, (I32)written, 0) < 0) {
        perror("send failed");
    }

    free(buffer);
}

I8* getUsername(const I8* message) {
    U16 offset = 1;
    while (message[offset - 1] != '!' || message[offset] != '~') offset++;

    I8* username = malloc(offset-1);
    for (U16 i = 0; i < offset-1; i++) username[i] = message[i+1];
    username[offset-2] = '\0';

    return username;
}

