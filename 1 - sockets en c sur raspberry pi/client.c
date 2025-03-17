#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Créer un socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Initialiser les informations de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Adresse IP du serveur
    server_address.sin_port = htons(PORT);

    // Se connecter au serveur
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Erreur lors de la connexion au serveur");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur sur le port %d\n", PORT);

    // Envoyer des messages au serveur depuis le clavier
    while (1) {
        printf("Entrez un message (ou 'exit' pour quitter) : ");
        fgets(buffer, sizeof(buffer), stdin);

        // Envoyer le message au serveur
        send(client_socket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit\n") == 0) {
            break; // Quitter la boucle si 'exit' est saisi
        }
    }

    // Fermer le socket
    close(client_socket);

    return 0;
}

