// proxy_server.h
#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <openssl/ssl.h>

// Initialise le proxy côté serveur
void proxy_init_server(int client_socket, SSL_CTX* ctx);

// Lance la boucle de réception (bloquante)
void proxy_run_server();

// Envoie un message au client
void proxy_send_to_client(const char* msg);

// Récupère le dernier message reçu du client (copie, à libérer)
char* proxy_recv_from_client();

// Nettoie tous les objets alloués
void proxy_cleanup_server();

#endif // PROXY_SERVER_H