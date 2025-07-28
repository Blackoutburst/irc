#include <openssl/ssl.h>
#include <openssl/err.h>

#include "irc.h"
#include "chat.h"

I32 ircRunCommand(ChatState* state, const I8* command) {
    if (!strncmp(command, "!list", 5)) { 
        IRC_LIST(); 
        return 1;
    }
    
    if (!strncmp(command, "!join ", 6)) { 
        memcpy(state->current_channel, command + 6, 250); 
        IRC_JOIN(command + 6);
        return 1;
    }
    
    return 0;
}

void ircProcessMessage(const I8* message) {
    ChatState* state = chatGetState();
    I8** words = stringSplit(message, ' ');

    // Echo keep connection alive
    if (!strncmp(words[0], "PING", 4)) {
        IRC_PONG(message + 5);
        
        stringCleanArray(words);
        return;
    }

    // User leave channel
    if (!strncmp(words[1], "QUIT", 4)) {
        I8* username = ircGetUsername(message);
        chatAddMessage("<<<", username);

        free(username);
        stringCleanArray(words);
        return;
    }

    // User join channel
    if (!strncmp(words[1], "JOIN", 4)) {
        I8* username = ircGetUsername(message);
        chatAddMessage(">>>", username);

        free(username);
        stringCleanArray(words);
        return;
    }

    // New message
    if (!strncmp(words[1], "PRIVMSG", 4)) {
        I8* username = ircGetUsername(message);
        
        U16 offset = 1;
        while (message[offset - 1] != ' ' || message[offset] != ':') offset++;

        chatAddMessage(username, message + offset + 1);

        free(username);
        stringCleanArray(words);
        return;
    }

    // Skipped packets
    if (!strncmp(words[1], "004", 3) 
        || !strncmp(words[1], "321", 3) 
        || !strncmp(words[1], "005", 3) 
        || !strncmp(words[1], "376", 3)
        || !strncmp(words[1], "323", 3)
        || !strncmp(words[1], "332", 3)
        || !strncmp(words[1], "333", 3)
        || !strncmp(words[1], "366", 3)
    ) {
        stringCleanArray(words);
        return;
    }

    // Update server name
    if (!strncmp(words[1], "002", 3)) {
        memcpy(state->server_name, words[6], strlen(words[6]) - 1); 
    }

    // Update server motd
    if (!strncmp(words[1], "372", 3)) {
        U16 offset = 1;
        while (message[offset - 1] != ' ' || message[offset] != ':') offset++;
        memcpy(state->server_motd, message + offset + 1, strlen(message + offset + 1)); 
    }

    // I don't remember
    if (!strncmp(words[1], "254", 3)) {
        I8 buffer[512] = { 0 };
    
        snprintf(buffer, 512, "%s %s %s", words[3], words[4], words[5]);

        chatAddMessage(state->server_name, buffer);

        stringCleanArray(words);
        return;
    }

    // Command list output
    if (!strncmp(words[1], "322", 3)) {
        I8 buffer[512] = { 0 };
    
        snprintf(buffer, 512, "- %s: %s", words[3], words[4]);

        chatAddMessage(state->server_name, buffer);
        
        stringCleanArray(words);
        return;
    }

    // Join message output used to update user list
    if (!strncmp(words[1], "353", 3)) {
        U16 offset = 1;
        while (message[offset - 1] != ' ' || message[offset] != ':') offset++;

        I8 buffer[512] = { 0 };
    
        snprintf(buffer, 512, "- %s: %s", words[4], message + offset + 1);

        I8** users = stringSplit(message + offset + 1, ' ');

        for (I32 i = 0; users[i]; i++) {
            strncpy(state->users[state->user_count], users[i], MAX_USERNAME_LEN - 1);
            state->users[state->user_count][MAX_USERNAME_LEN - 1] = '\0';
            state->user_count++;
        }
        
        stringCleanArray(users);
        
        chatAddMessage(state->server_name, buffer);
        
        stringCleanArray(words);
        return;
    }

    // Unprocessed data show as server messages
    U16 offset = 1;
    while (message[offset - 1] != ' ' || message[offset] != ':') offset++;
    I8 buffer[512] = { 0 };
    
    snprintf(buffer, 512, "%s", message + offset + 1);
    chatAddMessage(state->server_name, message + offset + 1);
    stringCleanArray(words);
}

I8* ircGetUsername(const I8* message) {
    U16 offset = 1;
    while (message[offset - 1] != '!' || message[offset] != '~') offset++;

    I8* username = malloc(offset-1);
    for (U16 i = 0; i < offset-1; i++) username[i] = message[i+1];
    username[offset-2] = '\0';

    return username;
}

void _ircSendPacket(const I8* format, ...) {
    SSL* ssl = clientGetSsl();

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

    if (SSL_write(ssl, buffer, (I32)written) < 0) {
        perror("send failed");
    }

    free(buffer);
}

// Packet functions

