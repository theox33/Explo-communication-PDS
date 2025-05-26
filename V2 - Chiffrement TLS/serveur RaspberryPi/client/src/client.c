/**
 * @file client.c
 * @author Your Name
 * @brief Client TLS principal pour communication sécurisée avec le serveur.
 * @date 2025-05-26
 * @license MIT
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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../../common/communication.h"
#include "../../common/connection.h"
#include "../../common/protocol.h"


#define SERVER_IP "127.0.0.1"
#define PORT 5001
#define BUFFER_SIZE 1024

Connection* connection = NULL;
Protocol* protocol = NULL;
Communication* communication = NULL;
int running = 1;

/**
 * @brief Callback pour gérer les messages reçus du serveur.
 * @param[in] cmd Commande reçue.
 * @param[in] param Paramètre associé à la commande.
 */
void message_handler(const char* cmd, const char* param) {
    printf("Received from server: %s\n", param);
    fflush(stdout);  // Ensure output is displayed immediately
}

/**
 * @brief Gestionnaire du signal d'interruption pour arrêt propre.
 * @param[in] sig Numéro du signal capturé.
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
 * @brief Point d'entrée de l'application client.
 * Initialise la connection TLS, le protocole et la communication,
 * puis lance la boucle principale.
 * @return Code de retour (0 si succès).
 */
int main() {
    char buffer[BUFFER_SIZE];
    signal(SIGINT, signal_handler);

    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // Création de l'objet Connection
    connection = Connection_create();
    if (!connection) {
        fprintf(stderr, "Failed to create Connection object\n");
        return 1;
    }

    // Création de l'objet Protocol
    protocol = Protocol_create();
    if (!protocol) {
        fprintf(stderr, "Failed to create Protocol object\n");
        Connection_destroy(connection);
        return 1;
    }

    // Création de l'objet Communication
    communication = Communication_create(connection, protocol);
    if (!communication) {
        fprintf(stderr, "Failed to create Communication object\n");
        Protocol_destroy(protocol);
        Connection_destroy(connection);
        return 1;
    }

    // Définition du gestionnaire de messages
    communication->setMessageHandler(communication, message_handler);

    // Connexion au serveur
    printf("Connecting to %s:%d...\n", SERVER_IP, PORT);
    connection->connect(connection, SERVER_IP, PORT);

    // --- SSL/TLS handshake ---
    if (connection->connected) {
        const SSL_METHOD *method = TLS_client_method();
        SSL_CTX *ctx = SSL_CTX_new(method);
        if (!ctx) {
            fprintf(stderr, "SSL_CTX_new failed\n");
            Communication_destroy(communication);
            Protocol_destroy(protocol);
            Connection_destroy(connection);
            return 1;
        }

        SSL *ssl = SSL_new(ctx);
        if (!ssl) {
            fprintf(stderr, "SSL_new failed\n");
            SSL_CTX_free(ctx);
            Communication_destroy(communication);
            Protocol_destroy(protocol);
            Connection_destroy(connection);
            return 1;
        }

        SSL_set_fd(ssl, connection->socket_fd);

        if (SSL_connect(ssl) <= 0) {
            fprintf(stderr, "SSL_connect failed\n");
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            Communication_destroy(communication);
            Protocol_destroy(protocol);
            Connection_destroy(connection);
            return 1;
        }

        printf("SSL connection established with cipher: %s\n", SSL_get_cipher(ssl));
        connection->ssl = ssl;
    }
    // --- End SSL/TLS handshake ---
    // Attendre un peu pour s'assurer que la connexion est établie
    sleep(1);
    
    // Vérifier si la connexion a réussi
    if (!connection->connected) {
        fprintf(stderr, "Failed to connect to server\n");
        Communication_destroy(communication);
        Protocol_destroy(protocol);
        Connection_destroy(connection);
        return 1;
    }
    
    printf("Connected to server successfully\n");

    // Démarrer le thread de communication
    communication->run(communication);
    
    // Envoi d'un message de test au serveur
    printf("Sending test message to server...\n");
    communication->comX(communication, "Test message from client");
    
    // Boucle principale pour lire les entrées de l'utilisateur
    printf("Enter messages (type 'exit' to quit):\n");

    printf("> ");
    
    while (running && connection->connected) {
        fflush(stdout);
        
        // Vérifier si la connexion est toujours active
        if (!connection->connected) {
            printf("Connection to server lost\n");
            break;
        }
        
        // Utiliser select pour attendre l'entrée de l'utilisateur avec un timeout
        struct timeval tv = {0, 100000}; // 100ms timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        // Vérifier si l'entrée est prête
        int select_result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        
        if (select_result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            // Si l'entrée est prête
            if (!fgets(buffer, BUFFER_SIZE, stdin)) {
                break;
            }
            
            // Supprimer le caractère de nouvelle ligne à la fin
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
            }
            
            // Vérifier si l'utilisateur veut quitter
            if (strcmp(buffer, "exit") == 0) {
                break;
            }
            
            // Envoyer le message au serveur
            if (connection->connected) {
                printf("Sending: %s\n", buffer);
                printf("> ");
                communication->comX(communication, buffer);
            } else {
                printf("Connection lost, cannot send message\n");
                break;
            }
        }

        usleep(10000);
    }

    printf("Shutting down client...\n");
    
    // Arrêter la communication et libérer les ressources
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