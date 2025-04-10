// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../../common/proxy_server.h"

#define PORT 12345
static int server_socket;

void signal_handler(int signum) {
    printf("\nArrêt du serveur...\n");
    close(server_socket);
    exit(0);
}

int main() {
    signal(SIGINT, signal_handler);

    // Initialisation OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_server_method());
    if (!ssl_ctx) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_certificate_file(ssl_ctx, "/etc/ssl/raspiserver/server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ssl_ctx, "/etc/ssl/raspiserver/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ssl_ctx);
        return 1;
    }

    // Création de la socket serveur
    struct sockaddr_in server_addr;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 1) < 0) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("accept");
        close(server_socket);
        return 1;
    }

    printf("Client connected: %s:%d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    proxy_init_server(client_socket, ssl_ctx);

    proxy_send_to_client("Bienvenue sur le serveur sécurisé !");

    proxy_run_server();

    proxy_cleanup_server();
    SSL_CTX_free(ssl_ctx);
    close(server_socket);
    return 0;
}