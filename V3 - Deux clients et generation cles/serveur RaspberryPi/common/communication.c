/**
 * @file communication.c
 * @brief Implementation of high-level communication interface for networked messaging
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #include "communication.h"
 #include <string.h>
 #include <unistd.h>
 #include <errno.h>
 
 /**
  * @brief Send a command X message with the specified parameter
  * 
  * Encodes a CMD_X message with the given parameter using the protocol
  * and sends it through the connection.
  * 
  * @param comm Pointer to the Communication object
  * @param param Parameter string to send with the command
  * 
  * @note The message is encoded using the protocol's encodeMessage function
  * @note The encoded message includes a null terminator in the transmission
  */
 static void Communication_comX(Communication* comm, const char* param) {
     char buffer[BUFFER_SIZE];
     comm->protocol->encodeMessage(comm->protocol, comm->protocol->cmdX, param, buffer);
     comm->connection->write(comm->connection, buffer, strlen(buffer) + 1);
 }
 
 /**
  * @brief Send a command Y message with the specified parameter
  * 
  * Encodes a CMD_Y message with the given parameter using the protocol
  * and sends it through the connection.
  * 
  * @param comm Pointer to the Communication object
  * @param param Parameter string to send with the command
  * 
  * @note The message is encoded using the protocol's encodeMessage function
  * @note The encoded message includes a null terminator in the transmission
  */
 static void Communication_comY(Communication* comm, const char* param) {
     char buffer[BUFFER_SIZE];
     comm->protocol->encodeMessage(comm->protocol, comm->protocol->cmdY, param, buffer);
     comm->connection->write(comm->connection, buffer, strlen(buffer) + 1);
 }
 
 /**
  * @brief Background thread function for continuous message reception
  * 
  * This function runs in a separate thread and continuously reads messages
  * from the connection, decodes them using the protocol, and calls the
  * message handler callback. It includes connection health monitoring
  * and graceful error handling.
  * 
  * @param arg Pointer to the Communication object (cast from void*)
  * @return void* Always returns NULL
  * 
  * @note The thread runs until comm->running is set to 0
  * @note Implements connection health monitoring with consecutive empty read detection
  * @note Uses a 50ms delay between read attempts to prevent CPU hogging
  * @note Performs connection probing after multiple consecutive empty reads
  */
 static void* communication_thread_function(void* arg) {
     Communication* comm = (Communication*)arg;
     char buffer[BUFFER_SIZE];
     char cmd[MAX_CMD_SIZE];
     char param[MAX_PARAM_SIZE];
     int consecutive_empty_reads = 0;
     const int max_consecutive_empty_reads = 5;
     
     printf("Communication thread started\n");
     
     while (comm->running) {
         // Clear buffers before each read
         memset(buffer, 0, BUFFER_SIZE);
         
         ssize_t bytes_read = comm->connection->read(comm->connection, buffer, BUFFER_SIZE - 1);
         
         if (bytes_read > 0) {
             // Reset empty read counter on successful data reception
             consecutive_empty_reads = 0;
             
             // Ensure null termination
             buffer[bytes_read] = '\0';
             printf("Raw data received: '%s'\n", buffer);
             
             // Clear command and parameter buffers
             memset(cmd, 0, MAX_CMD_SIZE);
             memset(param, 0, MAX_PARAM_SIZE);
             
             // Process the message
             comm->protocol->decodeMessage(comm->protocol, buffer, cmd, param);
             
             // Call the message handler if set
             if (comm->messageHandler) {
                 comm->messageHandler(cmd, param);
             }
         } else if (bytes_read == 0) {
             // No data available
             consecutive_empty_reads++;
             
             // Check if connection was marked as closed during read
             if (!comm->connection->connected) {
                 printf("Connection closed by peer (normal)\n");
                 break;
             }
             
             // If we've had too many consecutive empty reads, check connection
             if (consecutive_empty_reads > max_consecutive_empty_reads) {
                 // Try a zero-byte probe to test connection
                 char probe_buffer[1];
                 ssize_t probe_result = send(comm->connection->socket_fd, probe_buffer, 0, MSG_NOSIGNAL);
                 
                 if (probe_result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                     // Connection is likely broken
                     printf("Connection appears broken after multiple empty reads\n");
                     comm->connection->connected = 0;
                     break;
                 }
                 
                 // Reset counter after connection check
                 consecutive_empty_reads = 0;
             }
         } else {
             // Actual error occurred in read
             printf("Read error, terminating communication thread\n");
             break;
         }
         
         // Small delay to prevent CPU hogging on busy loop
         usleep(50000);  // 50ms delay
     }
     
     printf("Communication thread exiting\n");
     return NULL;
 }
 
 /**
  * @brief Start the communication thread for message reception
  * 
  * Sets the running flag and creates a background thread that will
  * continuously monitor the connection for incoming messages.
  * 
  * @param comm Pointer to the Communication object
  * 
  * @note The thread will run until stop() is called
  * @note Only one thread per Communication object is supported
  */
 static void Communication_run(Communication* comm) {
     comm->running = 1;
     pthread_create(&comm->thread, NULL, communication_thread_function, comm);
 }
 
 /**
  * @brief Stop the communication thread and wait for termination
  * 
  * Sets the running flag to 0, waits for the background thread to terminate,
  * and resets the thread handle.
  * 
  * @param comm Pointer to the Communication object
  * 
  * @note This function blocks until the thread terminates
  * @note It's safe to call this function multiple times
  */
 static void Communication_stop(Communication* comm) {
     comm->running = 0;
     if (comm->thread) {
         pthread_join(comm->thread, NULL);
         comm->thread = 0;
     }
 }
 
 /**
  * @brief Set the message handler callback function
  * 
  * Assigns a callback function that will be invoked whenever a message
  * is received and successfully decoded.
  * 
  * @param comm Pointer to the Communication object
  * @param handler Message handler callback function
  * 
  * @note The handler can be set to NULL to disable message processing
  * @note The handler is called from the communication thread context
  */
 static void Communication_setMessageHandler(Communication* comm, MessageHandler handler) {
     comm->messageHandler = handler;
 }
 
 /**
  * @brief Create a new Communication object
  * 
  * Allocates memory for a new Communication structure and initializes it
  * with the provided Connection and Protocol objects. All method pointers
  * are assigned for object-oriented usage.
  * 
  * @param connection Pointer to an initialized Connection object
  * @param protocol Pointer to an initialized Protocol object
  * @return Communication* Pointer to the newly created Communication object, or NULL on failure
  * 
  * @note The Connection and Protocol objects are stored by reference, not copied
  * @note The communication thread is not started automatically
  * @note All fields are initialized to safe default values
  */
 Communication* Communication_create(Connection* connection, Protocol* protocol) {
     Communication* comm = (Communication*)malloc(sizeof(Communication));
     if (comm) {
         comm->connection = connection;
         comm->protocol = protocol;
         comm->running = 0;
         comm->thread = 0;
         comm->messageHandler = NULL;
         
         // Assign method pointers
         comm->comX = Communication_comX;
         comm->comY = Communication_comY;
         comm->run = Communication_run;
         comm->stop = Communication_stop;
         comm->setMessageHandler = Communication_setMessageHandler;
     }
     return comm;
 }
 
 /**
  * @brief Destroy a Communication object and free its resources
  * 
  * Stops the communication thread if it's running, waits for termination,
  * and frees the allocated memory. The associated Connection and Protocol
  * objects are not destroyed.
  * 
  * @param comm Pointer to the Communication object to destroy
  * 
  * @note It's safe to pass NULL to this function
  * @note The Connection and Protocol objects are not freed as they may be shared
  * @note If the thread is running, it will be stopped before cleanup
  */
 void Communication_destroy(Communication* comm) {
     if (comm) {
         // Stop the communication thread
         if (comm->running) {
             comm->stop(comm);
         }
         
         // Don't destroy connection and protocol here as they might be shared
         free(comm);
     }
 }