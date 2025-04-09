#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./package/protocol/protobuf/src/protocol.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define INPUT_BUFFER_SIZE 1024

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

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Enlever le retour à la ligne
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            break;
        }

        size_t packet_size;
        uint8_t* packet = protocol_encrypt_message(input, &packet_size);

        ssize_t sent = write(sock, packet, packet_size);
        if (sent != (ssize_t)packet_size) {
            perror("Erreur envoi");
        } else {
            printf("Message envoyé (%ld octets).\n", sent);
        }

        free(packet);


        
    }

    close(sock);
    printf("Déconnexion du client.\n");
    return 0;
}
