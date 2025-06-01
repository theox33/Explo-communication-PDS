/**
 * @file communication.h
 * @brief High-level communication interface for networked messaging
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #ifndef COMMUNICATION_H
 #define COMMUNICATION_H
 
 #include <stdlib.h>
 #include <stdio.h>
 #include <pthread.h>
 #include "connection.h"
 #include "protocol.h"
 
 /**
  * @brief Function pointer type for message handling callbacks
  * 
  * This callback function is invoked when a message is received and decoded.
  * 
  * @param cmd The decoded command string
  * @param param The decoded parameter string
  */
 typedef void (*MessageHandler)(const char* cmd, const char* param);
 
 /**
  * @brief Communication structure for high-level message handling
  * 
  * This structure combines Connection and Protocol objects to provide
  * a high-level interface for sending and receiving messages over a network
  * connection. It manages a background thread for continuous message reception.
  */
 typedef struct Communication {
     Connection* connection;      /**< Network connection object */
     Protocol* protocol;          /**< Message protocol object */
     pthread_t thread;           /**< Background thread for message reception */
     int running;                /**< Thread running flag (1=running, 0=stopped) */
     MessageHandler messageHandler; /**< Callback function for received messages */
     
     // Method pointers for object-oriented interface
     /**
      * @brief Send a command X message with parameter
      * @param comm Pointer to the Communication object
      * @param param Parameter string to send with the command
      */
     void (*comX)(struct Communication*, const char*);
     
     /**
      * @brief Send a command Y message with parameter
      * @param comm Pointer to the Communication object
      * @param param Parameter string to send with the command
      */
     void (*comY)(struct Communication*, const char*);
     
     /**
      * @brief Start the communication thread for message reception
      * @param comm Pointer to the Communication object
      */
     void (*run)(struct Communication*);
     
     /**
      * @brief Stop the communication thread and wait for termination
      * @param comm Pointer to the Communication object
      */
     void (*stop)(struct Communication*);
     
     /**
      * @brief Set the message handler callback function
      * @param comm Pointer to the Communication object
      * @param handler Message handler callback function
      */
     void (*setMessageHandler)(struct Communication*, MessageHandler);
 } Communication;
 
 /**
  * @brief Create a new Communication object
  * 
  * Allocates memory for a new Communication structure and initializes it
  * with the provided Connection and Protocol objects.
  * 
  * @param connection Pointer to an initialized Connection object
  * @param protocol Pointer to an initialized Protocol object
  * @return Communication* Pointer to the newly created Communication object, or NULL on failure
  * 
  * @note The caller is responsible for calling Communication_destroy() to free resources
  * @note The Connection and Protocol objects are not owned by Communication
  * @note The message reception thread is not started automatically - call run() to start it
  */
 Communication* Communication_create(Connection* connection, Protocol* protocol);
 
 /**
  * @brief Destroy a Communication object and free its resources
  * 
  * Stops the communication thread if running, waits for it to terminate,
  * and frees the allocated memory. The associated Connection and Protocol
  * objects are not destroyed as they may be shared.
  * 
  * @param comm Pointer to the Communication object to destroy
  * 
  * @note It's safe to pass NULL to this function
  * @note The Connection and Protocol objects are not freed by this function
  * @note If the communication thread is running, it will be stopped first
  */
 void Communication_destroy(Communication* comm);
 
 #endif // COMMUNICATION_H