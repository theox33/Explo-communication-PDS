/**
 * @file connection.h
 * @brief Network connection management with SSL/TLS support
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

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
 
 /** @brief Default buffer size for network operations */
 #define BUFFER_SIZE 1024
 
 /**
  * @brief Connection structure for managing network connections
  * 
  * This structure encapsulates all the necessary components for managing
  * a network connection, including socket operations, SSL/TLS support,
  * and thread-safe operations.
  */
 typedef struct Connection {
     int socket_fd;              /**< Socket file descriptor */
     pthread_t thread;           /**< Background thread for connection management */
     int connected;              /**< Connection status flag (1=connected, 0=disconnected) */
     pthread_mutex_t mutex;      /**< Mutex for thread-safe operations */
     SSL* ssl;                   /**< SSL structure pointer for TLS communication */
     
     // Method pointers for object-oriented interface
     /**
      * @brief Connect to a remote server
      * @param conn Pointer to the Connection object
      * @param ip IP address of the server
      * @param port Port number of the server
      */
     void (*connect)(struct Connection*, const char*, int);
     
     /**
      * @brief Write data to the connection
      * @param conn Pointer to the Connection object
      * @param buffer Data buffer to write
      * @param length Number of bytes to write
      * @return ssize_t Number of bytes written, or -1 on error
      */
     ssize_t (*write)(struct Connection*, const void*, size_t);
     
     /**
      * @brief Read data from the connection
      * @param conn Pointer to the Connection object
      * @param buffer Buffer to store received data
      * @param length Maximum number of bytes to read
      * @return ssize_t Number of bytes read, 0 on connection close, -1 on error
      */
     ssize_t (*read)(struct Connection*, void*, size_t);
 } Connection;
 
 /**
  * @brief Create a new Connection object
  * 
  * Allocates memory for a new Connection structure and initializes all
  * fields to their default values. The connection is not established
  * until the connect() method is called.
  * 
  * @return Connection* Pointer to the newly created Connection object, or NULL on failure
  * 
  * @note The caller is responsible for calling Connection_destroy() to free resources
  * @note The socket is initialized to -1 (invalid) and connected flag to 0
  */
 Connection* Connection_create();
 
 /**
  * @brief Destroy a Connection object and free its resources
  * 
  * Closes the socket connection, cleans up SSL resources if present,
  * waits for background threads to terminate, and frees the allocated memory.
  * 
  * @param conn Pointer to the Connection object to destroy
  * 
  * @note It's safe to pass NULL to this function
  * @note This function will block until background threads terminate
  */
 void Connection_destroy(Connection* conn);
 
 #endif // CONNECTION_H