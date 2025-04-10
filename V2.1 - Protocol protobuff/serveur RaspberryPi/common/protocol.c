#include "protocol.h"
#include "./protobuf/dist/src/message.pb-c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define XOR_KEY 0x5A

static void xor_cipher(char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= XOR_KEY;
    }
}

uint8_t* Protocol_encodeMessage(const char* plain_text, size_t* out_len) {
    if (!plain_text || !out_len) return NULL;

    // Chiffrement
    char* encrypted = strdup(plain_text);
    xor_cipher(encrypted, strlen(encrypted));

    // Protobuf
    AMessage msg = AMESSAGE__INIT;
    msg.content = encrypted;

    size_t len = amessage__get_packed_size(&msg);
    uint8_t* buffer = malloc(len);
    amessage__pack(&msg, buffer);

    printf("[CLIENT] Message encodé (%zu bytes): ", len);
    for (size_t i = 0; i < *out_len; ++i) {
        printf("\\x%02X", buffer[i]);
    }
    printf("\n");


    *out_len = len;
    free(encrypted);
    return buffer;
}

char* Protocol_decodeMessage(const uint8_t* buffer, size_t len) {
    if (!buffer || len == 0) return NULL;

    // Désérialiser
    AMessage* msg = amessage__unpack(NULL, len, buffer);
    if (!msg) {
        fprintf(stderr, "Protobuf decode failed\n");
        return NULL;
    }

    // Déchiffrer
    char* decrypted = strdup(msg->content);
    xor_cipher(decrypted, strlen(decrypted));

    amessage__free_unpacked(msg, NULL);
    return decrypted;
}

struct Protocol {
    void* dummy;
};


static char* decode(Protocol* self, const uint8_t* buffer, size_t size) {
    (void)self;
    return Protocol_decodeMessage(buffer, size);
}

static uint8_t* encode(Protocol* self, const char* msg, size_t* size) {
    (void)self;
    return Protocol_encodeMessage(msg, size);
}

static void destroy(Protocol* self) {
    free(self);
}

Protocol* Protocol_create() {
    Protocol* self = malloc(sizeof(Protocol));
    return self;
}

void Protocol_destroy(Protocol* self) {
    if (self) free(self);
}

// Expose interface to Communication
ProtocolInterface Protocol_getInterface() {
    ProtocolInterface iface = {
        .encode = encode,
        .decode = decode,
        .destroy = destroy
    };
    return iface;
}
