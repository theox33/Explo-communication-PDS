// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CMD_SIZE 32
#define MAX_PARAM_SIZE 1024

typedef struct Protocol {
    const char* cmdX;
    const char* cmdY;
    
    // Methods (implemented as function pointers)
    void (*encodeMessage)(struct Protocol*, const char*, const char*, char*);
    void (*decodeMessage)(struct Protocol*, const char*, char*, char*);
} Protocol;

Protocol* Protocol_create();
void Protocol_destroy(Protocol* protocol);

#endif // PROTOCOL_H