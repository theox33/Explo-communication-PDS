#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stddef.h>
#include <sys/types.h>

typedef struct Connection Connection;
typedef struct Protocol Protocol;

typedef void (*MessageHandler)(const char* message);

typedef struct Communication {
    void (*run)(struct Communication* self);
    void (*stop)(struct Communication* self);
    void (*comX)(struct Communication* self, const char* msg);
    void (*setMessageHandler)(struct Communication* self, MessageHandler handler);

    // Données internes (opaque pour les autres modules)
    Connection* connection;
    Protocol* protocol;
    MessageHandler handler;
    int running;
} Communication;

// Constructeurs
Communication* Communication_create(Connection* connection, Protocol* protocol);
void Communication_destroy(Communication* communication);

// Fonctions de message (libres)
ssize_t send_message(int socket, const char* message);
char* receive_message(int socket);

#endif
