#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE 4
#endif

#ifndef TCP_KEEPINTVL
#define TCP_KEEPINTVL 5
#endif

#ifndef TCP_KEEPCNT
#define TCP_KEEPCNT 6
#endif

#include "connection.h"
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>

static void* connection_thread_function(void* arg) {
    Connection* conn = (Connection*)arg;
    // Thread can handle background tasks like heartbeat
    return NULL;
}

static void Connection_connect(Connection* conn, const char* ip, int port) {
    struct sockaddr_in server_address;
    
    pthread_mutex_lock(&conn->mutex);
    
    // Create socket if not already created
    if (conn->socket_fd <= 0) {
        conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (conn->socket_fd == -1) {
            perror("Socket creation failed");
            pthread_mutex_unlock(&conn->mutex);
            return;
        }

        // Enable TCP keep-alive
        int keepalive = 1;
        int keepidle = 60;  // Start probing after 60 seconds of inactivity
        int keepintvl = 10; // Send probe every 10 seconds
        int keepcnt = 5;    // Disconnect after 5 failed probes

        setsockopt(conn->socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    }
    
    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);
    
    // Connect to server
    if (connect(conn->socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        close(conn->socket_fd);
        conn->socket_fd = -1;
        conn->connected = 0;
        pthread_mutex_unlock(&conn->mutex);
        return;
    }
    
    conn->connected = 1;
    pthread_mutex_unlock(&conn->mutex);
    
    // Start background thread
    pthread_create(&conn->thread, NULL, connection_thread_function, conn);
}

static ssize_t Connection_write(Connection* conn, const void* buffer, size_t length) {
    ssize_t result = -1;
    pthread_mutex_lock(&conn->mutex);
    if (conn->connected && conn->socket_fd > 0) {
        if (conn->ssl) {
            result = SSL_write(conn->ssl, buffer, length);
        } else {
            result = send(conn->socket_fd, buffer, length, 0);
        }
        if (result < 0) {
            perror("Write failed");
            conn->connected = 0;
        }
    }
    pthread_mutex_unlock(&conn->mutex);
    return result;
}

static ssize_t Connection_read(Connection* conn, void* buffer, size_t length) {
    ssize_t result = -1;
    pthread_mutex_lock(&conn->mutex);
    if (conn->connected && conn->socket_fd > 0) {
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(conn->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        if (conn->ssl) {
            result = SSL_read(conn->ssl, buffer, length);
        } else {
            result = recv(conn->socket_fd, buffer, length, 0);
        }
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = 0;
            } else {
                perror("Read failed");
                conn->connected = 0;
            }
        } else if (result == 0) {
            conn->connected = 0;
        }
    }
    pthread_mutex_unlock(&conn->mutex);
    return result;
}


Connection* Connection_create() {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    if (conn) {
        conn->socket_fd = -1;
        conn->connected = 0;
        pthread_mutex_init(&conn->mutex, NULL);
        
        // Assign method pointers
        conn->connect = Connection_connect;
        conn->write = Connection_write;
        conn->read = Connection_read;
    }
    return conn;
}

void Connection_destroy(Connection* conn) {
    if (conn) {
        pthread_mutex_lock(&conn->mutex);
        if (conn->ssl) {
            SSL_shutdown(conn->ssl);
            SSL_free(conn->ssl);
            conn->ssl = NULL;
        }
        if (conn->socket_fd > 0) {
            close(conn->socket_fd);
            conn->socket_fd = -1;
        }
        conn->connected = 0;
        pthread_mutex_unlock(&conn->mutex);
        
        if (conn->thread) {
            pthread_join(conn->thread, NULL);
        }
        
        pthread_mutex_destroy(&conn->mutex);
        free(conn);
    }
}
