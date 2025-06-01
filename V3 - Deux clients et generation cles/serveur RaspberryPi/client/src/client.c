/**
 * @file client.c
 * @brief Interactive client application for connecting to the messaging server
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <signal.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <sys/time.h>  // Pour struct timeval
 #include <sys/select.h>  // Pour fd_set
 #include "../../common/communication.h"
 #include "../../common/connection.h"
 #include "../../common/protocol.h"
 
 /** @brief Default server IP address to connect to */
 #define SERVER_IP "172.23.3.21"  // Update to actual server IP
 
 /** @brief Default server port */
 #define PORT 5001
 
 /** @brief Buffer size for user input and messages */
 #define BUFFER_SIZE 1024
 
 /** @brief Global connection object pointer */
 Connection* connection = NULL;
 
 /** @brief Global protocol object pointer */
 Protocol* protocol = NULL;
 
 /** @brief Global communication object pointer */
 Communication* communication = NULL;
 
 /** @brief Client running flag (1=running, 0=shutting down) */
 int running = 1;
 
 /**
  * @brief Message handler callback for processing server messages
  * 
  * Called when a message is received from the server. Currently just
  * displays the received parameter to the console.
  * 
  * @param cmd The received command string (unused in current implementation)
  * @param param The received parameter string to display
  * 
  * @note The output is immediately flushed to ensure visibility
  */
 void message_handler(const char* cmd, const char* param) {
     printf("Received from server: %s\n", param);
     fflush(stdout);  // Ensure output is displayed immediately
 }
 
 /**
  * @brief Signal handler for graceful client shutdown
  * 
  * Handles SIGINT (Ctrl+C) signal to perform graceful client shutdown.
  * Stops communication, destroys all objects, and exits the application.
  * 
  * @param sig Signal number (expected to be SIGINT)
  * 
  * @note Sets running flag to 0
  * @note Stops and destroys communication object
  * @note Destroys protocol and connection objects
  * @note Calls exit(0) to terminate the process
  */
 void signal_handler(int sig) {
     printf("\nExiting client application...\n");
     running = 0;
     
     if (communication) {
         communication->stop(communication);
         Communication_destroy(communication);
         communication = NULL;
     }
     if (protocol) {
         Protocol_destroy(protocol);
         protocol = NULL;
     }
     if (connection) {
         Connection_destroy(connection);
         connection = NULL;
     }
     exit(0);
 }
 
 /**
  * @brief Main client function
  * 
  * Initializes the client application, connects to the server, and enters
  * the main input loop for interactive messaging. Handles user input with
  * non-blocking I/O to allow for connection monitoring.
  * 
  * @return int Exit status (0 for success, non-zero for failure)
  * 
  * @note Creates Connection, Protocol, and Communication objects
  * @note Connects to the server using SERVER_IP and PORT
  * @note Starts the communication thread for receiving messages
  * @note Sends an initial test message to the server
  * @note Enters interactive loop accepting user input
  * @note Uses select() for non-blocking input to monitor connection status
  * @note Gracefully handles connection loss and user exit commands
  * @note Cleans up all resources before terminating
  */
 int main() {
     char buffer[BUFFER_SIZE];
     signal(SIGINT, signal_handler);
 
     // Create connection object
     connection = Connection_create();
     if (!connection) {
         fprintf(stderr, "Failed to create Connection object\n");
         return 1;
     }
     
     // Create protocol object
     protocol = Protocol_create();
     if (!protocol) {
         fprintf(stderr, "Failed to create Protocol object\n");
         Connection_destroy(connection);
         return 1;
     }
     
     // Create communication object
     communication = Communication_create(connection, protocol);
     if (!communication) {
         fprintf(stderr, "Failed to create Communication object\n");
         Protocol_destroy(protocol);
         Connection_destroy(connection);
         return 1;
     }
     
     // Set message handler
     communication->setMessageHandler(communication, message_handler);
     
     // Connect to server
     printf("Connecting to %s:%d...\n", SERVER_IP, PORT);
     connection->connect(connection, SERVER_IP, PORT);
 
     // Wait for connection to establish
     sleep(1);
     
     // Check connection status
     if (!connection->connected) {
         fprintf(stderr, "Failed to connect to server\n");
         Communication_destroy(communication);
         Protocol_destroy(protocol);
         Connection_destroy(connection);
         return 1;
     }
     
     printf("Connected to server successfully\n");
 
     // Start communication thread
     communication->run(communication);
     
     // Send initial test message
     printf("Sending test message to server...\n");
     communication->comX(communication, "Test message from client");
     
     // Main input loop
     printf("Enter messages (type 'exit' to quit):\n");
 
     printf("> ");
     
     while (running && connection->connected) {
         fflush(stdout);
         
         // Check for disconnection before attempting to read input
         if (!connection->connected) {
             printf("Connection to server lost\n");
             break;
         }
         
         // Set up a non-blocking way to check for input
         struct timeval tv = {0, 100000}; // 100ms timeout
         fd_set readfds;
         FD_ZERO(&readfds);
         FD_SET(STDIN_FILENO, &readfds);
         
         // Check if input is available
         int select_result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
         
         if (select_result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
             // Input is available
             if (!fgets(buffer, BUFFER_SIZE, stdin)) {
                 break;
             }
             
             // Remove newline
             size_t len = strlen(buffer);
             if (len > 0 && buffer[len-1] == '\n') {
                 buffer[len-1] = '\0';
             }
             
             // Check for exit command
             if (strcmp(buffer, "exit") == 0) {
                 break;
             }
             
             // Send message
             if (connection->connected) {
                 printf("Sending: %s\n", buffer);
                 printf("> ");
                 communication->comX(communication, buffer);
             } else {
                 printf("Connection lost, cannot send message\n");
                 break;
             }
         }
         
         // Small delay to prevent CPU hogging
         usleep(10000);
     }
 
     printf("Shutting down client...\n");
     
     // Clean up resources
     if (communication) {
         communication->stop(communication);
         Communication_destroy(communication);
     }
     if (protocol) {
         Protocol_destroy(protocol);
     }
     if (connection) {
         Connection_destroy(connection);
     }
     
     return 0;
 }