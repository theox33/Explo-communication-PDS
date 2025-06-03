/**
 * @file tls_server.c
 * @brief TLS-secured chat server implementation
 * @version 4.0
 * @author Alexis DEVERCHERE
 *
 * @section description Description
 * This file implements a secure chat server using TLS 1.2+ encryption.
 * The server handles multiple concurrent clients with pthreads, provides
 * message broadcasting, and manages client nicknames.
 *
 * @section features Features
 * - Secure TLS 1.2+ communication
 * - Multi-client support (up to MAX_CLIENTS)
 * - Thread-safe client management
 * - Nickname support (/nick command)
 * - User listing (/list command)
 *
 * @section usage Usage
 * ./tls_server
 *
 * @section notes Notes
 * - Requires server.pem certificate file in certificates/ directory
 * - Uses mutexes for thread-safe client array access
 * - Listens on port 8443 by default
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8443
#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

/**
 * @struct Client
 * @brief Structure to store client connection information
 * 
 * Contains all necessary data to manage a connected client including
 * SSL context, socket, nickname, and activity status.
 */typedef struct {
    SSL *ssl;
    int socket;
    int id;
    char nickname[50];
    int active;
} Client;

// Variables globales
Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
SSL_CTX *ctx;
int server_running = 1;

/**
 * @brief Initialize OpenSSL library and load error strings
 * 
 * Prepares OpenSSL for server operations. Must be called before any SSL functions.
 */
void init_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

// Nettoyer OpenSSL
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * @brief Create and configure SSL context for server
 * @return SSL_CTX* Initialized SSL context
 * 
 * Creates server-side SSL context with TLS 1.2+ requirement.
 * Loads certificate and private key from server.pem.
 */
SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Forcer TLS 1.2 minimum
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    
    return ctx;
}

// Configurer le contexte SSL avec les certificats
void configure_context(SSL_CTX *ctx) {
    // Charger le certificat et la clé depuis le fichier PEM
    if (SSL_CTX_use_certificate_file(ctx, "certificates/server.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "certificates/server.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

// Initialiser les clients
void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        clients[i].ssl = NULL;
        clients[i].socket = -1;
        clients[i].id = i;
    }
}

// Ajouter un client
int add_client(SSL *ssl, int socket) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].ssl = ssl;
            clients[i].socket = socket;
            clients[i].active = 1;
            snprintf(clients[i].nickname, sizeof(clients[i].nickname), "Client%d", i);
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

// Retirer un client
void remove_client(int id) {
    pthread_mutex_lock(&clients_mutex);
    clients[id].active = 0;
    if (clients[id].ssl) {
        SSL_shutdown(clients[id].ssl);
        SSL_free(clients[id].ssl);
        clients[id].ssl = NULL;
    }
    if (clients[id].socket >= 0) {
        close(clients[id].socket);
        clients[id].socket = -1;
    }
    pthread_mutex_unlock(&clients_mutex);
}

/**
 * @brief Broadcast message to all connected clients
 * @param message Message to broadcast
 * @param sender_id ID of sending client (-1 for system messages)
 * 
 * Thread-safe function that sends a message to all active clients
 * except the sender (if specified).
 */
void broadcast_message(char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != sender_id) {
            if (SSL_write(clients[i].ssl, message, strlen(message)) <= 0) {
                // Erreur d'envoi, le client sera déconnecté par son thread
                ERR_print_errors_fp(stderr);
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Envoyer un message à un client spécifique
void send_to_client(int client_id, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    
    if (clients[client_id].active) {
        SSL_write(clients[client_id].ssl, message, strlen(message));
    }
    
    pthread_mutex_unlock(&clients_mutex);
}


/**
 * @brief Thread function to handle client communication
 * @param arg Pointer to client ID (cast to int*)
 * @return void* Always returns NULL
 * 
 * Manages all communication with a single client including:
 * - Message reception
 * - Command processing (/nick, /list, /quit)
 * - Broadcast to other clients
 * 
 * Runs in a separate thread for each client.
 */
void* handle_client(void *arg) {
    int client_id = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 100];
    int bytes;
    
    printf("Client %d connected\n", client_id);
    
    // Message de bienvenue
    snprintf(message, sizeof(message), "Welcome! You are %s. Type /nick <name> to change your nickname.\n", 
             clients[client_id].nickname);
    send_to_client(client_id, message);
    
    // Notifier les autres clients
    snprintf(message, sizeof(message), "%s has joined the chat\n", clients[client_id].nickname);
    broadcast_message(message, client_id);
    
    while (server_running && clients[client_id].active) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes = SSL_read(clients[client_id].ssl, buffer, BUFFER_SIZE - 1);
        
        if (bytes <= 0) {
            if (bytes < 0) {
                int err = SSL_get_error(clients[client_id].ssl, bytes);
                printf("SSL read error: %d\n", err);
            }
            break;
        }
        
        // Retirer le retour à la ligne
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        // Commandes spéciales
        if (strncmp(buffer, "/nick ", 6) == 0) {
            char old_nick[50];
            strcpy(old_nick, clients[client_id].nickname);
            
            pthread_mutex_lock(&clients_mutex);
            strncpy(clients[client_id].nickname, buffer + 6, sizeof(clients[client_id].nickname) - 1);
            pthread_mutex_unlock(&clients_mutex);
            
            snprintf(message, sizeof(message), "%s is now known as %s\n", old_nick, clients[client_id].nickname);
            broadcast_message(message, -1);
            printf("Client %d changed nickname to %s\n", client_id, clients[client_id].nickname);
            continue;
        }
        
        if (strcmp(buffer, "/list") == 0) {
            char list[BUFFER_SIZE] = "Active users:\n";
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    strcat(list, "- ");
                    strcat(list, clients[i].nickname);
                    if (i == client_id) strcat(list, " (you)");
                    strcat(list, "\n");
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            send_to_client(client_id, list);
            continue;
        }
        
        if (strcmp(buffer, "/quit") == 0) {
            break;
        }
        
        // Message normal - broadcaster à tous les autres clients
        printf("[%s]: %s\n", clients[client_id].nickname, buffer);
        snprintf(message, sizeof(message), "[%s]: %s\n", clients[client_id].nickname, buffer);
        broadcast_message(message, client_id);
    }
    
    // Déconnexion du client
    printf("Client %d (%s) disconnected\n", client_id, clients[client_id].nickname);
    snprintf(message, sizeof(message), "%s has left the chat\n", clients[client_id].nickname);
    broadcast_message(message, client_id);
    
    remove_client(client_id);
    return NULL;
}

// Gestionnaire de signal pour arrêt propre
void handle_signal(int sig) {
    printf("\nShutting down server...\n");
    server_running = 0;
}
/**
 * @brief Main server loop
 * @return int Exit status
 * 
 * Handles server initialization, client connection acceptance,
 * and thread management. Runs until SIGINT/SIGTERM received.
 */
int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Gérer les signaux
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialiser OpenSSL
    init_openssl();
    ctx = create_context();
    configure_context(ctx);
    
    // Initialiser les clients
    init_clients();
    
    // Créer le socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Permettre la réutilisation de l'adresse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("TLS Chat Server started on port %d\n", PORT);
    printf("Waiting for connections...\n");
    
    while (server_running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (server_running) {
                perror("Accept failed");
            }
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        // Créer une connexion SSL
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);
        
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_socket);
            continue;
        }
        
        // Ajouter le client
        int client_id = add_client(ssl, client_socket);
        if (client_id < 0) {
            printf("Server full, rejecting connection\n");
            SSL_write(ssl, "Server full\n", 12);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_socket);
            continue;
        }
        
        // Créer un thread pour gérer le client
        pthread_t thread;
        int *client_id_ptr = malloc(sizeof(int));
        *client_id_ptr = client_id;
        
        if (pthread_create(&thread, NULL, handle_client, client_id_ptr) != 0) {
            perror("Thread creation failed");
            remove_client(client_id);
            free(client_id_ptr);
            continue;
        }
        
        pthread_detach(thread);
    }
    
    // Nettoyer
    close(server_socket);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    
    return 0;
}