// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../../common/proxy_client.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

static int running = 1;


void signal_handler(int signum) {
    printf("\nFermeture...\n");
    running = 0;
    proxy_cleanup_client();
}

void message_handler(const char* message) {
    printf("[CLIENT] Message reçu du serveur : %s\n", message);
}

int main() {
    signal(SIGINT, signal_handler);

    if (!proxy_init_client(SERVER_IP, SERVER_PORT)) {
        fprintf(stderr, "Erreur d'initialisation du client\n");
        return 1;
    }

    proxy_send_to_server("Bonjour depuis le client!");

    // Lancer la réception continue en tâche de fond
    proxy_run_client();

    char buffer[1024];
    while (running) {
        printf("Vous: ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "exit") == 0) break;

        proxy_send_to_server(buffer);
    }

    proxy_cleanup_client();
    return 0;
}
