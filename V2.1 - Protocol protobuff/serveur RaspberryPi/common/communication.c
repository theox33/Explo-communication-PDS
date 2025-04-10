#include "communication.h"
#include "protocol.h"
#include "connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons, ntohs

#define MAX_MESSAGE_SIZE 8192

ssize_t send_message(int socket, const char* message) {
    if (!message) return -1;

    size_t encoded_len;
    uint8_t* encoded = Protocol_encodeMessage(message, &encoded_len);
    if (!encoded) return -1;

    if (encoded_len > UINT16_MAX) {
        fprintf(stderr, "Message too large to send\n");
        free(encoded);
        return -1;
    }

    uint16_t len_net = htons((uint16_t)encoded_len);  // 2 bytes length
    ssize_t sent = write(socket, &len_net, sizeof(len_net));
    if (sent != sizeof(len_net)) {
        perror("Error sending length");
        free(encoded);
        return -1;
    }

    sent = write(socket, encoded, encoded_len);
    if (sent != (ssize_t)encoded_len) {
        perror("Error sending message");
        free(encoded);
        return -1;
    }

    free(encoded);
    return sent;
}

char* receive_message(int socket) {
    uint16_t len_net;
    ssize_t received = read(socket, &len_net, sizeof(len_net));
    if (received == 0) return NULL; // Connection closed
    if (received != sizeof(len_net)) {
        perror("Error reading length");
        return NULL;
    }

    uint16_t message_len = ntohs(len_net);
    if (message_len > MAX_MESSAGE_SIZE) {
        fprintf(stderr, "Message too large (%d bytes)\n", message_len);
        return NULL;
    }

    uint8_t buffer[MAX_MESSAGE_SIZE];
    received = read(socket, buffer, message_len);
    if (received != message_len) {
        perror("Error reading full message");
        return NULL;
    }

    return Protocol_decodeMessage(buffer, message_len);
}

static void run_impl(Communication* self) {
    self->running = 1;

    while (self->running) {
        char* msg = receive_message(self->connection->socket_fd);
        if (!msg) break;

        if (self->handler) {
            self->handler(msg);
        } else {
            printf("Received: %s\n", msg);
        }

        free(msg);
    }

    printf("Communication loop ended.\n");
}

static void stop_impl(Communication* self) {
    self->running = 0;
}

static void comX_impl(Communication* self, const char* msg) {
    if (msg && self->connection) {
        send_message(self->connection->socket_fd, msg);
    }
}

static void set_handler_impl(Communication* self, MessageHandler handler) {
    self->handler = handler;
}

Communication* Communication_create(Connection* connection, Protocol* protocol) {
    Communication* self = malloc(sizeof(Communication));
    if (!self) return NULL;

    self->connection = connection;
    self->protocol = protocol;
    self->handler = NULL;
    self->running = 0;

    self->run = run_impl;
    self->stop = stop_impl;
    self->comX = comX_impl;
    self->setMessageHandler = set_handler_impl;

    return self;
}

void Communication_destroy(Communication* self) {
    if (!self) return;
    self->stop(self);
    free(self);
}