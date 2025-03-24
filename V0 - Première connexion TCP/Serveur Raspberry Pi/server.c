#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345       // Port d'écoute
#define MAX_BUF_SIZE 1024  // Taille du buffer pour les messages reçus

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[MAX_BUF_SIZE];
    int bytes_read;

    // Création de la socket TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Ecouter sur toutes les interfaces
    server_addr.sin_port = htons(PORT);       // Port d'écoute

    // Lier la socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erreur lors du bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Mise en écoute de la socket
    if (listen(server_fd, 1) == -1) {
        perror("Erreur lors de l'écoute");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Attente de connexion d'un client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
        perror("Erreur lors de l'acceptation de la connexion");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connecté : %s\n", inet_ntoa(client_addr.sin_addr));

    // Boucle de réception des messages du client (tablette)
    while ((bytes_read = read(client_fd, buffer, MAX_BUF_SIZE)) > 0) {
        buffer[bytes_read] = '\0';  // S'assurer que le message est terminé par un caractère nul
        printf("Message reçu : %s\n", buffer);
    }

    if (bytes_read == -1) {
        perror("Erreur lors de la lecture");
    }

    printf("Déconnexion du client.\n");

    // Fermeture des sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
