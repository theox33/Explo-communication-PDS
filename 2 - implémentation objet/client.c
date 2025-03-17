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
#include "communication.h"
#include "connection.h"
#include "protocol.h"

#define SERVER_IP "172.17.0.1"  // Update to actual server IP
#define PORT 5001
#define BUFFER_SIZE 1024

Connection* connection = NULL;
Protocol* protocol = NULL;
Communication* communication = NULL;
int running = 1;

void message_handler(const char* cmd, const char* param) {
    printf("Received from server: %s\n", param);
    fflush(stdout);  // Ensure output is displayed immediately
}

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