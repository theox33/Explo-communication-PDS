#include "protocol.h"
#include ".././dist/src/message.pb-c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define XOR_KEY 0x5A

void protocol_init(void) {}

static void xor_cipher(char* data, size_t len) {
    for (size_t i = 0; i < len; ++i)
        data[i] ^= XOR_KEY;
}

uint8_t* protocol_encrypt_message(const char* plain_text, size_t* out_size) {
    // Copier et chiffrer le message
    char* encrypted = strdup(plain_text);
    xor_cipher(encrypted, strlen(encrypted));

    // Affichage du message chiffré avant Protobuf
    printf("Encrypted (XORed) message: ");
    for (size_t i = 0; i < strlen(encrypted); i++) {
        printf("\\x%02X", (unsigned char)encrypted[i]);
    }
    printf("\n");

    // Créer le message protobuf
    AMessage msg = AMESSAGE__INIT;
    msg.content = encrypted;

    // Sérialiser
    size_t size = amessage__get_packed_size(&msg);
    uint8_t* buffer = malloc(size);
    amessage__pack(&msg, buffer);

    *out_size = size;
    free(encrypted);
    return buffer;
}


char* protocol_decrypt_message(const uint8_t* data, size_t size) {
    // Affiche les octets bruts reçus
    printf("Received serialized data (%ld bytes): ", size);
    for (size_t i = 0; i < size; ++i) {
        printf("\\x%02X", data[i]);
    }
    printf("\n");

    // Désérialiser le message protobuf
    AMessage* msg = amessage__unpack(NULL, size, data);
    if (!msg) {
        fprintf(stderr, "Erreur : échec du décompactage protobuf\n");
        return NULL;
    }

    // Affiche le message chiffré contenu dans content
    printf("Encrypted message (protobuf content): ");
    for (size_t i = 0; i < strlen(msg->content); ++i) {
        printf("\\x%02X", (unsigned char)msg->content[i]);
    }
    printf("\n");

    // Déchiffrer le contenu
    char* decrypted = strdup(msg->content);
    xor_cipher(decrypted, strlen(decrypted));

    // Affiche le résultat final
    printf("Decrypted message: %s\n", decrypted);

    amessage__free_unpacked(msg, NULL);
    return decrypted;
}

