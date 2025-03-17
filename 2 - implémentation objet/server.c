// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "communication.h"
#include "connection.h"
#include "protocol.h"

#define PORT 5001
#define MAX_CLIENTS 10

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    int id;
} ClientInfo;

Connection* server_connections[MAX_CLIENTS] = {NULL};
Communication* server_communications[MAX_CLIENTS] = {NULL};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;
int running_server = 1;

// Message handler with improved debugging
void message_handler(const char* cmd, const char* param) {
    pthread_t tid = pthread_self();
    
    // Log the received message with thread ID for debugging
    printf("Thread %lu received: Command='%s', Param='%s'\n", 
           (unsigned long)tid, cmd, param);
    fflush(stdout);

    if (strcmp(cmd, "CMD_X") == 0) {
        printf("Client says: %s\n", param);
        
        // Echo message back to all clients (optional)
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_communications[i]) {
                server_communications[i]->comY(server_communications[i], param);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    } else if (strcmp(cmd, "CMD_Y") == 0) {
        printf("Command Y received: %s\n", param);
    } else {
        printf("Unknown command received: '%s'\n", cmd);
    }
    fflush(stdout);
}

// Client handler thread function
void* client_handler(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_socket = client_info->client_socket;
    int client_id = client_info->id;
    
    printf("Client %d connected from %s:%d\n", 
           client_id, 
           inet_ntoa(client_info->client_addr.sin_addr), 
           ntohs(client_info->client_addr.sin_port));
    
    // Create connection object
    Connection* connection = Connection_create();
    connection->socket_fd = client_socket;
    connection->connected = 1;
    
    // Create protocol object
    Protocol* protocol = Protocol_create();
    
    // Create communication object
    Communication* communication = Communication_create(connection, protocol);
    communication->setMessageHandler(communication, message_handler);
    
    // Store references
    pthread_mutex_lock(&clients_mutex);
    server_connections[client_id] = connection;
    server_communications[client_id] = communication;
    pthread_mutex_unlock(&clients_mutex);
    
    // Start the communication
    communication->run(communication);
    
    // Send welcome message
    printf("Sending welcome message to client %d\n", client_id);
    communication->comX(communication, "Welcome to the server!");
    
    // Monitor connection state rather than joining thread
    while (connection->connected && running_server) {
        sleep(1);
    }
    
    // Clean up
    pthread_mutex_lock(&clients_mutex);
    printf("Cleaning up resources for client %d\n", client_id);
    communication->stop(communication);
    Communication_destroy(communication);
    Protocol_destroy(protocol);
    Connection_destroy(connection);
    server_connections[client_id] = NULL;
    server_communications[client_id] = NULL;
    pthread_mutex_unlock(&clients_mutex);
    
    printf("Client %d disconnected\n", client_id);
    free(client_info);
    return NULL;
}

// SIGINT handler for clean shutdown
void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nServer shutting down...\n");
    running_server = 0;

    // Close server socket to interrupt accept()
    if (server_socket > 0) {
        close(server_socket);
    }
    
    // Clean up all client connections
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server_communications[i]) {
            server_communications[i]->stop(server_communications[i]);
            Communication_destroy(server_communications[i]);
            server_communications[i] = NULL;
        }
        if (server_connections[i]) {
            Connection_destroy(server_connections[i]);
            server_connections[i] = NULL;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    exit(EXIT_SUCCESS);
}

int main() {
    signal(SIGINT, signal_sigint_handler);
    struct sockaddr_in server_address;
    
    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    // Bind socket to address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    // Start listening for connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\nWaiting for client connections...\n", PORT);
    fflush(stdout);
    
    // Main loop - accept and handle client connections
    while (running_server) {
        // Allocate client info structure
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        if (!client_info) {
            perror("Memory allocation failed");
            continue;
        }
        
        // Accept connection
        socklen_t client_addr_len = sizeof(client_info->client_addr);
        client_info->client_socket = accept(server_socket, 
                                           (struct sockaddr*)&client_info->client_addr, 
                                           &client_addr_len);
        
        if (client_info->client_socket == -1) {
            if (errno == EINTR) {
                // Interrupted by signal, check if we should continue
                if (!running_server) {
                    free(client_info);
                    break;
                }
                continue;
            }
            
            perror("Accept failed");
            free(client_info);
            continue;
        }
        
        // Find available client slot
        pthread_mutex_lock(&clients_mutex);
        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_connections[i] == NULL) {
                slot = i;
                break;
            }
        }
        
        if (slot == -1) {
            // No slots available
            pthread_mutex_unlock(&clients_mutex);
            printf("Server full, rejecting connection\n");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
        
        client_info->id = slot;
        pthread_mutex_unlock(&clients_mutex);
        
        // Create thread to handle client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, client_info) != 0) {
            perror("Thread creation failed");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
        
        // Detach thread so resources are automatically released
        pthread_detach(client_thread);
    }
    
    close(server_socket);
    return 0;
}