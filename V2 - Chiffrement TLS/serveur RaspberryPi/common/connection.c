#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE 4
#endif

#ifndef TCP_KEEPINTVL
#define TCP_KEEPINTVL 5
#endif

#ifndef TCP_KEEPCNT
#define TCP_KEEPCNT 6
#endif

#include "connection.h"
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>

// Le thread peut gérer des tâches en arrière-plan comme le heartbeat
static void* connection_thread_function(void* arg) {
    Connection* conn = (Connection*)arg;
    return NULL;
}

static void Connection_connect(Connection* conn, const char* ip, int port) {
    struct sockaddr_in server_address;
    
    pthread_mutex_lock(&conn->mutex);
    
    // Créer le socket s'il n'est pas déjà créé
    if (conn->socket_fd <= 0) {
        conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (conn->socket_fd == -1) {
            perror("Échec de la création du socket");
            pthread_mutex_unlock(&conn->mutex);
            return;
        }

        // Activer le keep-alive TCP
        int keepalive = 1;
        int keepidle = 60;  // Commencer la sonde après 60 secondes d'inactivité
        int keepintvl = 10; // Envoyer une sonde toutes les 10 secondes
        int keepcnt = 5;    // Déconnecter après 5 sondes échouées

        // Configurer les options de keep-alive
        setsockopt(conn->socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
        setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    }
    
    // Configurer l'adresse du serveur
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);
    
    // Se connecter au serveur
    if (connect(conn->socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Échec de la connexion");
        close(conn->socket_fd);
        conn->socket_fd = -1;
        conn->connected = 0;
        pthread_mutex_unlock(&conn->mutex);
        return;
    }
    
    conn->connected = 1;
    pthread_mutex_unlock(&conn->mutex);
    
    // Démarrer le thread en arrière-plan
    pthread_create(&conn->thread, NULL, connection_thread_function, conn);
}

static ssize_t Connection_write(Connection* conn, const void* buffer, size_t length) {
    ssize_t result = -1;
    pthread_mutex_lock(&conn->mutex);
    if (conn->connected && conn->socket_fd > 0) {
        if (conn->ssl) {
            result = SSL_write(conn->ssl, buffer, length);
        } else {
            result = send(conn->socket_fd, buffer, length, 0);
        }
        if (result < 0) {
            perror("Échec de l'écriture");
            conn->connected = 0;
        }
    }
    pthread_mutex_unlock(&conn->mutex);
    return result;
}

static ssize_t Connection_read(Connection* conn, void* buffer, size_t length) {
    ssize_t result = -1;
    pthread_mutex_lock(&conn->mutex);
    if (conn->connected && conn->socket_fd > 0) {
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(conn->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        if (conn->ssl) {
            result = SSL_read(conn->ssl, buffer, length);
        } else {
            result = recv(conn->socket_fd, buffer, length, 0);
        }
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = 0;
            } else {
                perror("Échec de la lecture");
                conn->connected = 0;
            }
        } else if (result == 0) {
            conn->connected = 0;
        }
    }
    pthread_mutex_unlock(&conn->mutex);
    return result;
}

Connection* Connection_create() {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    if (conn) {
        conn->socket_fd = -1;
        conn->connected = 0;
        pthread_mutex_init(&conn->mutex, NULL);
        
        // Assigner les pointeurs de méthode
        conn->connect = Connection_connect;
        conn->write = Connection_write;
        conn->read = Connection_read;
    }
    return conn;
}

void Connection_destroy(Connection* conn) {
    if (conn) {
        pthread_mutex_lock(&conn->mutex);
        if (conn->ssl) {
            SSL_shutdown(conn->ssl);
            SSL_free(conn->ssl);
            conn->ssl = NULL;
        }
        if (conn->socket_fd > 0) {
            close(conn->socket_fd);
            conn->socket_fd = -1;
        }
        conn->connected = 0;
        pthread_mutex_unlock(&conn->mutex);
        
        if (conn->thread) {
            pthread_join(conn->thread, NULL);
        }
        
        pthread_mutex_destroy(&conn->mutex);
        free(conn);
    }
}