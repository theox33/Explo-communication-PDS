/**
 * @file protocol.h
 * @brief Interface du protocole Protobuf pour la sérialisation et le chiffrement des messages.
 * @author Théo AVRIL
 * @license MIT
 *
 * Ce module fournit des fonctions pour initialiser le protocole, chiffrer (sérialiser) un message texte
 * en format Protobuf, et déchiffrer (désérialiser) un message reçu.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialise le système de protocole (à utiliser si nécessaire).
 */
void protocol_init(void);

/**
 * @brief Chiffre et sérialise un message texte en format Protobuf.
 *
 * Cette fonction prend une chaîne de caractères en clair, la sérialise au format Protobuf,
 * et retourne un buffer alloué dynamiquement contenant le message encodé.
 *
 * @param[in] plain_text Le message texte à encoder.
 * @param[out] out_size Pointeur vers la taille du buffer retourné.
 * @return Pointeur vers le buffer alloué dynamiquement (à libérer avec free).
 */
uint8_t* protocol_encrypt_message(const char* plain_text, size_t* out_size);

/**
 * @brief Désérialise et déchiffre un message Protobuf reçu.
 *
 * Cette fonction prend un buffer contenant un message Protobuf, le désérialise,
 * et retourne une chaîne de caractères contenant le texte clair.
 *
 * @param[in] data Buffer contenant le message Protobuf.
 * @param[in] size Taille du buffer.
 * @return Pointeur vers la chaîne de texte clair (à libérer avec free).
 */
char* protocol_decrypt_message(const uint8_t* data, size_t size);

#endif // PROTOCOL_H