/**
 * @file protocol.h
 * @author Théo AVRIL
 * @brief Encodage/décodage des messages pour le projet TLS.
 * @date 2025-05-26
 * @license MIT
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CMD_SIZE 32
#define MAX_PARAM_SIZE 1024

/**
 * @struct Protocol
 * @brief Définition du format de message (commande + param).
 */
typedef struct Protocol {
    const char* cmdX; //Commande de type X
    const char* cmdY; //Commande de type Y
    
    // Méthodes de traitement des messages
    /**
     * @brief Encode un message en format "CMD:PARAM".
     * @param[in,out] protocol Instance de protocole.
     * @param[in] cmd Commande à encoder.
     * @param[in] param Paramètre associé à la commande.
     * @param[out] buffer Buffer pour stocker le message encodé.
     */
    void (*encodeMessage)(struct Protocol*, const char*, const char*, char*);
    /**
     * @brief Décode un message en séparant la commande et le paramètre.
     * @param[in,out] protocol Instance de protocole.
     * @param[in] message Message à décoder.
     * @param[out] cmd Buffer pour stocker la commande extraite.
     * @param[out] param Buffer pour stocker le paramètre extrait.
     */
    void (*decodeMessage)(struct Protocol*, const char*, char*, char*);
} Protocol;

/**
 * @brief Crée et configure le protocole.
 * @return Pointeur sur Protocol.
 */
Protocol* Protocol_create();

/**
 * @brief Détruit le protocole.
 * @param[in,out] protocol Instance de protocole.
 */
void Protocol_destroy(Protocol* protocol);

#endif // PROTOCOL_H