// proxy_client.h
#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

// Initialise la connexion client et prépare TLS + Communication
int proxy_init_client(const char* ip, int port);

// Envoie un message au serveur
void proxy_send_to_server(const char* msg);

// Reçoit le dernier message du serveur (copie à libérer)
char* proxy_recv_from_server();

// Lance la boucle de réception continue
void proxy_run_client();

// Libère tous les objets alloués
void proxy_cleanup_client();

#endif // PROXY_CLIENT_H