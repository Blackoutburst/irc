#include "chat.h"

ChatState* state = NULL;

ChatState* chatGetState(void) {
    return state;
}

void chatInit(void) {
    state = malloc(sizeof(ChatState));

    state->input_len = 0;
    state->message_count = 0;
    state->user_count = 0;
    state->input[0] = '\0';
    state->current_channel[0] = '\0';
    state->server_name[0] = '\0';
    state->server_motd[0] = '\0';
    state->username[0] = '\0';
}

void chatAddMessage(const I8* username, const I8* message) {
    strncpy(state->messages[state->message_count].username, username, MAX_USERNAME_LEN - 1);
    strncpy(state->messages[state->message_count].content, message, MAX_MESSAGE_LEN - 1);
    
    state->messages[state->message_count].username[MAX_USERNAME_LEN - 1] = '\0';
    state->messages[state->message_count].content[MAX_MESSAGE_LEN - 1] = '\0';
    state->message_count++;
}
