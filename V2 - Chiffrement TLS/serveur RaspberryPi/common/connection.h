// connection.h
#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 1024

typedef struct Connection {
    int socket_fd;
    pthread_t thread;
    int connected;
    pthread_mutex_t mutex;
    SSL* ssl; // Pointer to the SSL structure for TLS communication
    
    // Methods
    void (*connect)(struct Connection*, const char*, int);
    ssize_t (*write)(struct Connection*, const void*, size_t);
    ssize_t (*read)(struct Connection*, void*, size_t);
} Connection;

Connection* Connection_create();
void Connection_destroy(Connection* conn);

#endif // CONNECTION_H