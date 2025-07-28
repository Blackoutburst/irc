#include "client.h"
#include "ui.h"

static I32 sockfd = -1;
static pthread_t thread;
static SSL_CTX* ctx = NULL;
static SSL* ssl = NULL;

I32 clientConnect(const I8* ip, U16 port) {
    struct sockaddr_in server_addr;

    if (ctx == NULL) {
        _initOpenssl();
        ctx = SSL_CTX_new(TLS_client_method());
        
        if (ctx == NULL) {
            printf("SSL context creation failed\n");
            ERR_print_errors_fp(stderr);
            return 0;
        }
    }

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

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        return 0;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        printf("SSL creation failed\n");
        ERR_print_errors_fp(stderr);
        return 0;
    }
    
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) <= 0) {
        printf("SSL handshake failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
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
    I32 bytes_received = 0;

    while (1) {
        I8 byte[1];
        I32 ret = SSL_read(ssl, byte, 1);
        if (ret <= 0) {
            I32 err = SSL_get_error(ssl, ret);
            
            if (err == SSL_ERROR_ZERO_RETURN) {
                printf("Server closed connection\n");
            } else {
                printf("SSL read failed\n");
                ERR_print_errors_fp(stderr);
            }
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            sockfd = -1;
            break;
        }

        recv_buffer[bytes_received] = byte[0];
        bytes_received += ret;

        if (bytes_received > 1 && recv_buffer[bytes_received - 2] == '\r' && recv_buffer[bytes_received - 1] == '\n') {
            recv_buffer[bytes_received - 2] = '\0';
            ircProcessMessage(recv_buffer);
            uiUpdate(1);
            bytes_received = 0;
        }

        if (bytes_received >= BUFFER_SIZE) {
            printf("Buffer overflow\n");
            break;
        }
    }

    return NULL;
}

void clientClose(void) {
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
    }
    
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
    
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
        ctx = NULL;
    }
}

I32 clientGetfd(void) {
    return sockfd;
}

SSL* clientGetSsl(void) {
    return ssl;
}

pthread_t clientGetThread(void) {
    return thread;
}

void _initOpenssl(void) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}
