#define TB_IMPL

#include "irc.h"
#include "ui.h"
#include "client.h"
#include "chat.h"

int main(int argc, char** argv) {
    if (argc < 5) {
        printf("Error use ./irc <ip> <port> <nick> <real>\n");
        return 1;
    }

    chatInit();
    
    ChatState* state = chatGetState();
    memcpy(state->server_name, "IRC", 3); 
    memcpy(state->username, argv[3], strlen(argv[3])); 
    
    if (!uiInit()) return 1;

    clientConnect(argv[1], atoi(argv[2]));
    
    I32 sockfd = clientGetfd();
    pthread_t thread = clientGetThread();

    while (sockfd != -1) {
        uiUpdate();
    }

    pthread_join(thread, NULL);

    uiClean();

    clientClose();

    return 0;
}

