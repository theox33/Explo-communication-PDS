/**
 * @file connection.h
 * @author Théo AVRIL
 * @brief Gestion de la connexion TLS via OpenSSL.
 * @date 2025-05-26
 * @license MIT
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 1024

/**
 * @struct Connection
 * @brief Encapsulation d'une connexion réseau sécurisée TLS.
 */
typedef struct Connection {
    int socket_fd;  /**< Descripteur de socket pour la connexion réseau */
    pthread_t thread;   /**< Thread d'écoute pour la connexion */
    int connected;  /**< Indicateur de connexion active */
    pthread_mutex_t mutex;  /**< Mutex pour la synchronisation des accès */
    SSL* ssl; /**< Pointeur vers la structure SSL pour la connexion sécurisée */
    
    // Méthodes de connexion
    void (*connect)(struct Connection*, const char*, int);  /**< Établit la connexion TLS */
    ssize_t (*write)(struct Connection*, const void*, size_t);  /**< Envoie des données via TLS */
    ssize_t (*read)(struct Connection*, void*, size_t); /**< Lit des données via TLS */
} Connection;

/**
 * @brief Crée une connexion TLS non-initialisée.
 * @return Pointeur sur Connection.
 */
Connection* Connection_create();

/**
 * @brief Détruit la connexion et libère SSL.
 * @param[in,out] conn Instance de connexion.
 */
void Connection_destroy(Connection* conn);

#endif // CONNECTION_H