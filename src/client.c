#include "client.h"

static I32 sockfd = -1;
static pthread_t thread;

I32 clientConnect(const I8* ip, U16 port) {
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Socket creation failed\n");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        printf("Invalid server address\n");
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        return 0;
    }

    if (pthread_create(&thread, NULL, clientRead, NULL)) {
        printf("Network thread creation failed\n");
        return 0;
    }

    return 1;
}

void* clientRead(void* arg) {
    I8 recv_buffer[BUFFER_SIZE];

    while (1) {
        I32 bytes_received = 0;
        I8 byte[1];
        
        while (1) {
            bytes_received += recv(sockfd, byte, 1, 0);
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Server closed connection\n");
                } else {
                    perror("recv failed");
                }
                close(sockfd);
                sockfd = -1;
                break;
            }
            recv_buffer[bytes_received - 1] = byte[0];
            if (bytes_received > 1 && recv_buffer[bytes_received - 2] == '\r' && recv_buffer[bytes_received - 1] == '\n') {
                recv_buffer[bytes_received - 2] = '\0';
                ircProcessMessage(recv_buffer);
                break;
            }
        }
    }

    return NULL;
}

void clientClose(void) {
    close(sockfd);
}

I32 clientGetfd(void) {
    return sockfd;
}

pthread_t clientGetThread(void) {
    return thread;
}
