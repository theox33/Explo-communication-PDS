// proxy_server.c
#include "proxy_server.h"
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

static void server_message_handler(const char* message) {
    if (last_received_message) {
        free(last_received_message);
    }
    last_received_message = strdup(message);
    printf("[proxy_server] Message du client : %s\n", message);
}

void proxy_init_server(int client_socket, SSL_CTX* ctx) {
    connection = Connection_create();
    Connection_setSSL(connection, ctx);
    connection->socket_fd = client_socket;

    protocol = Protocol_create();
    communication = Communication_create(connection, protocol);
    communication->setMessageHandler(communication, server_message_handler);
}

void proxy_run_server() {
    if (communication) {
        communication->run(communication);
    }
}

void proxy_send_to_client(const char* msg) {
    if (communication) {
        communication->comX(communication, msg);
    }
}

char* proxy_recv_from_client() {
    return last_received_message ? strdup(last_received_message) : NULL;
}

void proxy_cleanup_server() {
    if (communication) Communication_destroy(communication);
    if (protocol) Protocol_destroy(protocol);
    if (connection) Connection_destroy(connection);
    if (last_received_message) free(last_received_message);
    communication = NULL;
    protocol = NULL;
    connection = NULL;
    last_received_message = NULL;
}
