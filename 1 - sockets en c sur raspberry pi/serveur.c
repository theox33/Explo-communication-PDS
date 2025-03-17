#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024


int server_socket, client_socket;
int running_server = 1;

void signal_sigint_handler(int signal)
{
    fprintf(stdout, "\nSIGINT intercepté (PID %d)\n", getpid());
    close(client_socket);
    exit(EXIT_SUCCESS);
}

void gestion_client(int client_socket)
{
    int running_client = 1;
    char buffer[BUFFER_SIZE];
    static int id_client = 0;

    id_client++;

    while(running_client)
    {
        ssize_t bytes_received;    
        bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received == -1) {
            perror("Erreur lors de la lecture depuis le socket");
            break;
        }
        else if(bytes_received == 0)
        {
            printf("Deconnexion du client %d \n", id_client );
            running_client = 0;
        }
        else
        {
            if (strcmp(buffer, "exit\n") == 0)
            {
                running_client = 0;
            }
            else
            {
                buffer[bytes_received] = '\0'; // Ajouter le caractère de fin de chaîne
                printf("Message reçu du client %d: %s\n",id_client, buffer);
            }
        }
    }
    
    printf("Fin client\n");

}

int main() {

    /* interception du Ctrl+C pour sortie propre */
	signal(SIGINT, signal_sigint_handler);
    
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    

    // Créer un socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Initialiser les informations de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Lier le socket à l'adresse et au port spécifiés
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Erreur lors de la liaison du socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Mettre en attente de connexions entrantes
    if (listen(server_socket, 5) == -1) {
        perror("Erreur lors de la mise en écoute du socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }


    while(running_server)
    {

        printf("Serveur en attente de connexions sur le port %d...\n", PORT);

        // Accepter la connexion d'un client
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        printf("Connexion acceptée de %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        gestion_client(client_socket);

        close(client_socket);
  
    }
    close(server_socket);

    return 0;
}


