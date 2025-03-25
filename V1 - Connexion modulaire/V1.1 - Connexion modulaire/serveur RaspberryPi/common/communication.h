// communication.h
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "connection.h"
#include "protocol.h"

typedef void (*MessageHandler)(const char* cmd, const char* param);

typedef struct Communication {
    Connection* connection;
    Protocol* protocol;
    pthread_t thread;
    int running;
    MessageHandler messageHandler;
    
    // Methods
    void (*comX)(struct Communication*, const char*);
    void (*comY)(struct Communication*, const char*);
    void (*run)(struct Communication*);
    void (*stop)(struct Communication*);
    void (*setMessageHandler)(struct Communication*, MessageHandler);
} Communication;

Communication* Communication_create(Connection* connection, Protocol* protocol);
void Communication_destroy(Communication* comm);

#endif // COMMUNICATION_H