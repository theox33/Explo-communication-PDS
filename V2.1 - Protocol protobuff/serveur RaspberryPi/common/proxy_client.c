// proxy_client.c
#include "proxy_client.h"
#include "communication.h"
#include "connection.h"
#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Communication* communication = NULL;
static Connection* connection = NULL;
static Protocol* protocol = NULL;

static char* last_received_message = NULL;

static void client_message_handler(const char* message) {
    if (last_received_message) {
        free(last_received_message);
    }
    last_received_message = strdup(message);
    printf("[proxy_client] Message du serveur : %s\n", message);
}

int proxy_init_client(const char* ip, int port) {
    connection = Connection_create();

    // TLS client setup
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Erreur création contexte TLS client\n");
        return 0;
    }
    Connection_setSSL(connection, ctx);

    connection->connect(connection, ip, port);

    protocol = Protocol_create();
    communication = Communication_create(connection, protocol);
    communication->setMessageHandler(communication, client_message_handler);

    return 1;
}

void proxy_send_to_server(const char* msg) {
    if (communication) {
        communication->comX(communication, msg);
    }
}

char* proxy_recv_from_server() {
    return last_received_message ? strdup(last_received_message) : NULL;
}

void proxy_run_client() {
    if (communication) {
        communication->run(communication);
    }
}

void proxy_cleanup_client() {
    if (communication) Communication_destroy(communication);
    if (protocol) Protocol_destroy(protocol);
    if (connection) Connection_destroy(connection);
    if (last_received_message) free(last_received_message);
    communication = NULL;
    protocol = NULL;
    connection = NULL;
    last_received_message = NULL;
}
