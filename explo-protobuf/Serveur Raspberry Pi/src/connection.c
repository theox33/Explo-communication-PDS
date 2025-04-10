#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <termios.h>


#include "./package/protocol/protobuf/src/protocol.h"
#include "./package/protocol/protobuf/dist/src/message.pb-c.h"

#define PORT 12345
#define BUFFER_SIZE 1024

int server_socket, client_socket;
int running_server = 1;

typedef struct {
    int socket;
    int id_client;
} send_thread_args;


void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nSIGINT intercepted (PID %d)\n", getpid());
    close(client_socket);
    exit(EXIT_SUCCESS);
}

void* thread_envoi_serveur(void* arg) {
    send_thread_args* args = (send_thread_args*)arg;
    int socket = args->socket;
    int id_client = args->id_client;
    free(arg);

    char input_buffer[BUFFER_SIZE];

    while (1) {
        printf("Saisir un message à envoyer au client %d (ou 'exit') :\n> ", id_client);
        fflush(stdout);

        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            break;
        }

        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        if (strcmp(input_buffer, "exit") == 0) {
            break;
        }

        size_t packet_size;
        uint8_t* packet = protocol_encrypt_message(input_buffer, &packet_size);
        ssize_t sent = write(socket, packet, packet_size);

        if (sent < 0) {
            perror("Erreur envoi serveur -> client");
        } else {
            printf("Serveur → Client %d : message envoyé (%ld octets)\n", id_client, sent);
        }

        free(packet);
    }

    pthread_exit(NULL);
}


void gestion_client(int client_socket) {
    int running_client = 1;
    char buffer[BUFFER_SIZE];
    static int id_client = 0;

    id_client++;

    // Créer une thread pour l'envoi serveur -> client
    pthread_t send_thread;
    send_thread_args* args = malloc(sizeof(send_thread_args));
    args->socket = client_socket;
    args->id_client = id_client;

    if (pthread_create(&send_thread, NULL, thread_envoi_serveur, args) != 0) {
        perror("Erreur création thread d'envoi");
    }


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
    pthread_cancel(send_thread);
    pthread_join(send_thread, NULL);

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
