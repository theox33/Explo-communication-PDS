#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *response = "Message reçu par le serveur C";
    
    // Création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Échec de création du socket");
        exit(EXIT_FAILURE);
    }
    
    // Configuration des options du socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("Échec setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Liaison du socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Échec du bind");
        exit(EXIT_FAILURE);
    }
    
    // Écoute des connexions
    if (listen(server_fd, 3) < 0) {
        perror("Échec du listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Serveur C en écoute sur le port %d...\n", PORT);
    
    while (1) {
        // Accepter une connexion
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Échec de l'accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Connexion acceptée depuis %s:%d\n", 
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        
        // Lire le message du client
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            printf("Message reçu: %s\n", buffer);
            
            // Envoyer une réponse
            send(new_socket, response, strlen(response), 0);
            printf("Réponse envoyée\n");
        }
        
        // Fermer la connexion
        close(new_socket);
        memset(buffer, 0, BUFFER_SIZE);
    }
    
    close(server_fd);
    return 0;
}