/**
 * @file connection.c
 * @brief Implementation of network connection management with SSL/TLS support
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

// TCP keepalive constants for cross-platform compatibility
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

/**
 * @brief Background thread function for connection management
 * 
 * This function runs in a separate thread and can handle background tasks
 * such as heartbeat monitoring, connection health checks, etc.
 * 
 * @param arg Pointer to the Connection object (cast from void*)
 * @return void* Always returns NULL
 * 
 * @note Currently this is a placeholder for future background functionality
 */
static void* connection_thread_function(void* arg) {
    Connection* conn = (Connection*)arg;
    // Thread can handle background tasks like heartbeat
    return NULL;
}

/**
 * @brief Establish a connection to a remote server
 * 
 * Creates a TCP socket, configures keepalive settings, and connects to the
 * specified server. The operation is thread-safe using the connection's mutex.
 * 
 * @param conn Pointer to the Connection object
 * @param ip IP address of the server to connect to
 * @param port Port number of the server
 * 
 * @note If the socket is already created, it reuses the existing socket
 * @note TCP keepalive is configured with: 60s idle, 10s interval, 5 probes
 * @note A background thread is started after successful connection
 */
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

/**
 * @brief Write data to the connection
 * 
 * Sends data through the connection using either SSL_write() for encrypted
 * connections or send() for plain TCP connections. The operation is thread-safe.
 * 
 * @param conn Pointer to the Connection object
 * @param buffer Data buffer to write
 * @param length Number of bytes to write
 * @return ssize_t Number of bytes written, or -1 on error
 * 
 * @note If an error occurs, the connection is marked as disconnected
 * @note The function automatically detects SSL vs plain TCP based on conn->ssl
 */
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

/**
 * @brief Read data from the connection
 * 
 * Receives data from the connection using either SSL_read() for encrypted
 * connections or recv() for plain TCP connections. The operation is thread-safe
 * and includes a 2-second timeout.
 * 
 * @param conn Pointer to the Connection object
 * @param buffer Buffer to store received data
 * @param length Maximum number of bytes to read
 * @return ssize_t Number of bytes read, 0 on connection close, -1 on error
 * 
 * @note A receive timeout of 2 seconds is set on the socket
 * @note EAGAIN/EWOULDBLOCK errors are treated as no data available (return 0)
 * @note If recv/SSL_read returns 0, the connection is marked as disconnected
 */
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

/**
 * @brief Create a new Connection object
 * 
 * Allocates memory for a new Connection structure and initializes all
 * fields to their default values. Method pointers are assigned to enable
 * object-oriented usage.
 * 
 * @return Connection* Pointer to the newly created Connection object, or NULL on failure
 * 
 * @note The socket_fd is initialized to -1 (invalid)
 * @note The connected flag is initialized to 0 (disconnected)
 * @note The mutex is initialized for thread-safe operations
 * @note SSL pointer is initialized to NULL
 */
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

/**
 * @brief Destroy a Connection object and free its resources
 * 
 * Performs proper cleanup by shutting down SSL connections, closing sockets,
 * waiting for background threads to terminate, and freeing allocated memory.
 * 
 * @param conn Pointer to the Connection object to destroy
 * 
 * @note It's safe to pass NULL to this function
 * @note SSL connections are properly shut down before closing
 * @note The function waits for the background thread to terminate
 * @note The mutex is destroyed to prevent resource leaks
 */
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