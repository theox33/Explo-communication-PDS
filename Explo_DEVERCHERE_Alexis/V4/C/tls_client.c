/**
 * @file tls_client.c
 * @brief TLS-secured chat client implementation
 * @version 4.0
 * @author Alexis DEVERCHERE
 *
 * @section description Description
 * This file implements a secure chat client using TLS 1.2+ encryption.
 * The client connects to a server, establishes a TLS session, and provides
 * real-time messaging with command support (/nick, /list, /help, etc.).
 *
 * @section features Features
 * - Secure TLS 1.2+ communication
 * - Asynchronous message reception
 * - Interactive terminal interface
 * - Server certificate verification (optional)
 * - Command processing
 *
 * @section usage Usage
 * ./tls_client <server_ip> <port>
 *
 * @section notes Notes
 * - For testing purposes, certificate verification is disabled by default
 * - Uses OpenSSL for TLS implementation
 * - Requires server.pem certificate file in production mode
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <termios.h>

#define BUFFER_SIZE 4096

// Variables globales
SSL *ssl;
int connected = 1;
pthread_t receive_thread;

/**
 * @brief Initialize OpenSSL library and load error strings
 * 
 * This function initializes the OpenSSL library, loads all algorithms,
 * and prepares error strings. Must be called before any other OpenSSL operations.
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
 * @brief Create and configure SSL context for client
 * @return SSL_CTX* Initialized SSL context
 * 
 * Creates a client-side SSL context with TLS 1.2 as minimum protocol version.
 * Exits program on failure.
 */
SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method();
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

// Configurer le contexte pour accepter les certificats auto-signés
void configure_context(SSL_CTX *ctx) {
    // Pour les tests avec certificats auto-signés, désactiver la vérification
    // ATTENTION: Ne pas utiliser en production!
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    // Alternative plus sûre: charger le certificat du serveur
    /*
    if (SSL_CTX_load_verify_locations(ctx, "certificates/server.crt", NULL) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    */
}

/**
 * @brief Thread function for receiving messages from server
 * @param arg Unused parameter (required by pthread interface)
 * @return void* Always returns NULL
 * 
 * Continuously reads incoming messages from the SSL connection and displays them.
 * Runs in a separate thread to allow simultaneous sending/receiving.
 */
void* receive_messages(void *arg) {
    (void)arg; // Éviter l'avertissement de paramètre non utilisé
    char buffer[BUFFER_SIZE];
    int bytes;
    
    while (connected) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
        
        if (bytes <= 0) {
            if (bytes < 0) {
                int err = SSL_get_error(ssl, bytes);
                if (err != SSL_ERROR_ZERO_RETURN) {
                    fprintf(stderr, "\nSSL read error: %d\n", err);
                    ERR_print_errors_fp(stderr);
                }
            }
            printf("\nDisconnected from server\n");
            connected = 0;
            break;
        }
        
        // Afficher le message reçu
        printf("\r%s", buffer);
        printf("> ");
        fflush(stdout);
    }
    
    return NULL;
}

// Configurer le terminal pour un mode plus interactif
void setup_terminal() {
    // Désactiver le buffering pour une entrée plus réactive
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
}

// Restaurer les paramètres du terminal
void restore_terminal() {
    printf("\n");
}

// Gestionnaire de signal
void handle_signal(int sig) {
    (void)sig;
    printf("\nDisconnecting...\n");
    connected = 0;
}

// Afficher l'aide
void print_help() {
    printf("\n=== TLS Chat Client ===\n");
    printf("Commands:\n");
    printf("  /nick <name>  - Change your nickname\n");
    printf("  /list         - List active users\n");
    printf("  /help         - Show this help\n");
    printf("  /quit         - Disconnect from server\n");
    printf("  /clear        - Clear screen\n");
    printf("========================\n\n");
}
/**
 * @brief Main client loop
 * @param argc Argument count
 * @param argv Argument vector [server_ip, port]
 * @return int Exit status
 * 
 * Handles connection establishment, SSL setup, and main input loop.
 * Manages cleanup on exit.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    int socket_fd;
    struct sockaddr_in server_addr;
    SSL_CTX *ctx;
    
    // Gérer les signaux
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Configurer le terminal
    setup_terminal();
    
    // Initialiser OpenSSL
    init_openssl();
    ctx = create_context();
    configure_context(ctx);
    
    // Créer le socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    
    // Se connecter au serveur
    printf("Connecting to %s:%d...\n", server_ip, port);
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    // Créer la connexion SSL
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_fd);
    
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
    
    // Afficher les informations du certificat
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        char *line;
        printf("Server certificate:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("  Subject: %s\n", line);
        free(line);
        X509_free(cert);
    }
    
    print_help();
    
    // Démarrer le thread de réception
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Boucle principale pour l'envoi de messages
    char buffer[BUFFER_SIZE];
    printf("> ");
    
    while (connected && fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Retirer le retour à la ligne
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Commandes locales
        if (strcmp(buffer, "/help") == 0) {
            print_help();
            printf("> ");
            continue;
        }
        
        if (strcmp(buffer, "/clear") == 0) {
            system("clear");
            printf("> ");
            continue;
        }
        
        // Envoyer le message au serveur
        if (strlen(buffer) > 0) {
            if (SSL_write(ssl, buffer, strlen(buffer)) <= 0) {
                ERR_print_errors_fp(stderr);
                break;
            }
            
            // Si c'est /quit, on se déconnecte
            if (strcmp(buffer, "/quit") == 0) {
                connected = 0;
                break;
            }
        }
        
        if (connected) {
            printf("> ");
        }
    }
    
    // Attendre la fin du thread de réception
    connected = 0;
    pthread_join(receive_thread, NULL);
    
    // Nettoyer
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socket_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    restore_terminal();
    
    printf("Goodbye!\n");
    return 0;
}