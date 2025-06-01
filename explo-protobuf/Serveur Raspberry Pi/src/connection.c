/**
 * @file connection.c
 * @brief Serveur TCP utilisant Protobuf pour la communication avec des clients.
 *
 * Ce programme implémente un serveur TCP (Hermes) capable d'accepter des connexions de clients (Ares),
 * d'envoyer et de recevoir des messages encodés avec Protobuf. L'utilisateur peut saisir des messages
 * à envoyer à chaque client connecté via le terminal. La communication est gérée dans des threads séparés
 * pour permettre l'envoi et la réception simultanés.
 *
 * - Le serveur écoute sur le port 12345.
 * - À chaque connexion client, un thread est créé pour permettre l'envoi de messages du serveur vers le client.
 * - Les messages reçus sont décodés avec Protobuf et affichés.
 * - La gestion du signal SIGINT permet une fermeture propre de la connexion.
 *
 * @author Théo AVRIL
 * @license MIT
 */

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
#define BUFFER_SIZE 256

int server_socket, client_socket;
int running_server = 1;

/**
 * @struct send_thread_args
 * @brief Arguments pour le thread d'envoi de messages serveur -> client.
 */
typedef struct {
    int socket;      /**< Socket du client */
    int id_client;   /**< Identifiant du client */
} send_thread_args;

/**
 * @brief Handler du signal SIGINT pour fermer proprement la connexion.
 * @param signal Numéro du signal reçu.
 */
void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nSIGINT intercepted (PID %d)\n", getpid());
    close(client_socket);
    exit(EXIT_SUCCESS);
}

/**
 * @brief Thread permettant à l'utilisateur d'envoyer des messages à un client.
 * @param arg Pointeur vers une structure send_thread_args.
 * @return NULL.
 */
void* postMessage(void* arg) {
    send_thread_args* args = (send_thread_args*)arg;
    int socket = args->socket;      // Récupère la socket du client
    int id_client = args->id_client; // Récupère l'ID du client
    free(arg); // Libère la mémoire allouée pour les arguments du thread

    char input_buffer[BUFFER_SIZE];

    while (1) {
        // Invite l'utilisateur à saisir un message à envoyer au client
        printf("Saisir un message à envoyer a Ares %d (ou 'exit') :\n> ", id_client);
        fflush(stdout);

        // Lecture de la saisie utilisateur
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            break; // Fin de fichier ou erreur d'entrée
        }

        // Suppression du retour à la ligne
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        // Si l'utilisateur tape "exit", on quitte la boucle d'envoi
        if (strcmp(input_buffer, "exit") == 0) {
            break;
        }

        // Encodage du message avec Protobuf
        size_t packet_size;
        uint8_t* packet = protocol_encrypt_message(input_buffer, &packet_size);

        // Envoi du message encodé au client via la socket
        ssize_t sent = write(socket, packet, packet_size);

        if (sent < 0) {
            perror("Erreur envoi Hermes -> Ares"); // Affiche une erreur si l'envoi échoue
        } else {
            printf("Hermes → Ares %d : message envoyé (%ld octets)\n", id_client, sent); // Confirmation d'envoi
        }

        free(packet); // Libère la mémoire allouée pour le message encodé
    }

    pthread_exit(NULL); // Termine proprement le thread
}

/**
 * @brief Fonction principale de gestion de la communication avec un client.
 *        Lance un thread pour l'envoi, et gère la réception des messages.
 * @param client_socket Socket du client connecté.
 */
void receiveMessage(int client_socket) {
    int running_client = 1;
    char buffer[BUFFER_SIZE];
    static int id_client = 0;

    id_client++;

    // Créer une thread pour l'envoi serveur -> client
    pthread_t send_thread;
    send_thread_args* args = malloc(sizeof(send_thread_args));
    args->socket = client_socket;
    args->id_client = id_client;

    if (pthread_create(&send_thread, NULL, postMessage, args) != 0) {
        perror("Erreur création thread d'envoi");
    }

    // Boucle de réception des messages du client
    while (running_client) {
        ssize_t bytes_received;
        bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received == -1) {
            perror("Error reading from socket");
            break;
        } else if (bytes_received == 0) {
            printf("Ares %d disconnected\n", id_client);
            running_client = 0;
        } else {
            buffer[bytes_received] = '\0';  // Termine la chaîne reçue
            if (strcmp(buffer, "exit\n") == 0) {
                running_client = 0;
            } else {
                size_t msg_size = bytes_received;
                char* plain = protocol_decrypt_message((uint8_t*)buffer, msg_size);
                if (plain) {
                    printf("Received from Ares %d: %s\n", id_client, plain);
                    free(plain);
                } else {
                    fprintf(stderr, "Failed to decode protobuf message\n");
                }
            }
        }
    }
    
    printf("Ares %d session ended\n", id_client);
    pthread_cancel(send_thread);
    pthread_join(send_thread, NULL);

}

/**
 * @brief Point d'entrée principal du serveur Hermes.
 *        Gère l'initialisation, l'écoute, l'acceptation des connexions et la gestion des clients.
 * @return 0 en cas de succès, code d'erreur sinon.
 */
int main() {

    // Intercepte Ctrl+C pour fermer la connexion proprement
    signal(SIGINT, signal_sigint_handler);
    
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    
    // Création de la socket TCP
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // Configuration de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    // Liaison de la socket à l'adresse et au port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    // Mise en écoute des connexions entrantes (jusqu'à 5 en attente)
    if (listen(server_socket, 5) == -1) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    while (running_server) {
        printf("Hermes waiting for connections on port %d...\n", PORT);

        // Accepte une connexion client
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Gère la session avec le client
        receiveMessage(client_socket);
        
        // Ferme la socket client après la session
        close(client_socket);
    }
    
    close(server_socket);
    return 0;
}