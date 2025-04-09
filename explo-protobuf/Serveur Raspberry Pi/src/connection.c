#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "./package/protocol/protobuf/src/protocol.h"
#include "./package/protocol/protobuf/dist/src/message.pb-c.h"

#define PORT 12345
#define BUFFER_SIZE 1024

int server_socket, client_socket;
int running_server = 1;

void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nSIGINT intercepted (PID %d)\n", getpid());
    close(client_socket);
    exit(EXIT_SUCCESS);
}

void gestion_client(int client_socket) {
    int running_client = 1;
    char buffer[BUFFER_SIZE];
    static int id_client = 0;

    id_client++;

    while (running_client) {
        ssize_t bytes_received;
        bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received == -1) {
            perror("Error reading from socket");
            break;
        } else if (bytes_received == 0) {
            printf("Client %d disconnected\n", id_client);
            running_client = 0;
        } else {
            buffer[bytes_received] = '\0';  // Null-terminate the string
            if (strcmp(buffer, "exit\n") == 0) {
                running_client = 0;
            } else {
                size_t msg_size = bytes_received;
                char* plain = protocol_decrypt_message((uint8_t*)buffer, msg_size);
                if (plain) {
                    printf("Received from client %d: %s\n", id_client, plain);
                    free(plain);
                } else {
                    fprintf(stderr, "Failed to decode protobuf message\n");
                }
            }
        }
    }
    
    printf("Client %d session ended\n", id_client);
}

int main() {

    // Intercept Ctrl+C to close the connection gracefully
    signal(SIGINT, signal_sigint_handler);
    
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    
    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // Configure the server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    // Bind the socket to the server address and port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections (max 5 pending connections)
    if (listen(server_socket, 5) == -1) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    while (running_server) {
        printf("Server waiting for connections on port %d...\n", PORT);

        // Accept a client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Process the connection with the client
        gestion_client(client_socket);
        
        // Close the client socket after handling the session
        close(client_socket);
    }
    
    close(server_socket);
    return 0;
}
