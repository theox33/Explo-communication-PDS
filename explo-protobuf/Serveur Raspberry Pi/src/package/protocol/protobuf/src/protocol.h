#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

// Initialise le système de protocole (si besoin plus tard)
void protocol_init(void);

// Chiffre un message, le sérialise, et renvoie un buffer alloué dynamiquement
uint8_t* protocol_encrypt_message(const char* plain_text, size_t* out_size);

// Déchiffre un message reçu (format protobuf), et renvoie le texte clair
char* protocol_decrypt_message(const uint8_t* data, size_t size);

#endif // PROTOCOL_H
