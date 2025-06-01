/**
 * @file communication.h
 * @author Théo AVRIL
 * @brief Gestion de la communication TLS sécurisée (traitement des types de messages X et Y).
 * @date 2025-05-26
 * @license MIT
 */
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "connection.h"
#include "protocol.h"

/**
 * @typedef MessageHandler
 * @brief Prototype de fonction pour traitement de messages.
 * @param[in] cmd Commande reçue.
 * @param[in] param Contenu du message.
 * @param[in] sender_id Identifiant de l'émetteur.
 */
typedef void (*MessageHandler)(const char* cmd, const char* param, int sender_id);

/**
 * @struct Communication
 * @brief Contexte pour la gestion TLS de bout en bout.
 */
typedef struct Communication {
    Connection* connection;   /**< Connexion TLS */
    Protocol* protocol;       /**< Protocole de traitement des messages */
    pthread_t thread;         /**< Thread d'écoute */
    MessageHandler messageHandler; /**< Callback pour chaque message */
    int client_id;            /**< Identifiant client */
    int running;              /**< Indicateur de fonctionnement du thread */
    
    // Méthodes de communication
    void (*com)(struct Communication*, const char*); /**< Envoi de message */
    void (*comX)(struct Communication*, const char*); /**< Envoi de message de type X avec traitement spécifique */
    void (*comY)(struct Communication*, const char*); /**< Envoi de message de type Y avec traitement spécifique */
    void (*run)(struct Communication*); /**< Démarre le thread de communication */
    void (*stop)(struct Communication*); /**< Arrête le thread de communication */
    void (*setMessageHandler)(struct Communication* comm, MessageHandler handler); /**< Définit le gestionnaire de messages */
} Communication;

/**
 * @brief Crée et initialise la structure de communication.
 * @param[in] connection Connexion TLS déjà établie.
 * @param[in] protocol Protocole configuré.
 * @return Pointeur sur Communication initialisée.
 */
Communication* Communication_create(Connection* connection, Protocol* protocol);

/**
 * @brief Arrête et libère la structure Communication.
 * @param[in,out] comm Instance de communication.
 */
void Communication_destroy(Communication* comm);

#endif // COMMUNICATION_H