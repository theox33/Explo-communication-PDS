#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

// API bas-niveau
uint8_t* Protocol_encodeMessage(const char* plain_text, size_t* out_len);
char* Protocol_decodeMessage(const uint8_t* buffer, size_t len);

// API orientée objet
typedef struct Protocol Protocol;

typedef struct {
    uint8_t* (*encode)(Protocol* self, const char* msg, size_t* size);
    char* (*decode)(Protocol* self, const uint8_t* buffer, size_t size);
    void (*destroy)(Protocol* self);
} ProtocolInterface;

Protocol* Protocol_create();
void Protocol_destroy(Protocol* self);
ProtocolInterface Protocol_getInterface();

#endif
