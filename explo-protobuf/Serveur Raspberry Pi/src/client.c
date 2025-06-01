/**
 * @file client.c
 * @brief Client TCP utilisant Protobuf pour envoyer des messages à un serveur.
 *
 * Ce programme se connecte à un serveur TCP, lit des messages depuis l'entrée standard,
 * les encode avec Protobuf via la fonction protocol_encrypt_message, puis les envoie au serveur.
 * L'utilisateur peut saisir "exit" pour fermer la connexion proprement.
 *
 * @author Théo AVRIL
 * @date 2025-06-01
 * @license MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./package/protocol/protobuf/src/protocol.h"

#define SERVER_IP "127.0.0.1"           /**< Adresse IP du serveur */
#define SERVER_PORT 12345               /**< Port du serveur */
#define INPUT_BUFFER_SIZE 1024          /**< Taille du buffer d'entrée utilisateur */

/**
 * @brief Point d'entrée du client TCP.
 *
 * - Crée une socket TCP et se connecte au serveur.
 * - Lit les messages de l'utilisateur.
 * - Encode chaque message avec Protobuf.
 * - Envoie le message encodé au serveur.
 * - Quitte si l'utilisateur tape "exit".
 *
 * @return 0 en cas de succès, 1 en cas d'erreur.
 */
int main() {
    int sock;
    struct sockaddr_in server_addr;
    char input[INPUT_BUFFER_SIZE];

    // Créer la socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        return 1;
    }

    // Définir l’adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur connexion");
        close(sock);
        return 1;
    }

    printf("Connecté au serveur !\n");
    printf("Tapez un message (\"exit\" pour quitter) :\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        // Lecture de l'entrée utilisateur
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Fin de fichier ou erreur d'entrée
        }

        // Enlever le retour à la ligne à la fin de la saisie
        input[strcspn(input, "\n")] = '\0';

        // Si l'utilisateur tape "exit", on quitte la boucle
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Encodage du message utilisateur avec Protobuf
        size_t packet_size;
        uint8_t* packet = protocol_encrypt_message(input, &packet_size);

        // Envoi du message encodé au serveur via la socket TCP
        ssize_t sent = write(sock, packet, packet_size);
        if (sent != (ssize_t)packet_size) {
            perror("Erreur envoi"); // Affiche une erreur si l'envoi est incomplet
        } else {
            printf("Message envoyé (%ld octets).\n", sent); // Confirmation d'envoi
        }

        free(packet); // Libération de la mémoire allouée pour le message encodé
    }

    // Fermeture de la socket et message de déconnexion
    close(sock);
    printf("Déconnexion du client.\n");
    return 0;
}