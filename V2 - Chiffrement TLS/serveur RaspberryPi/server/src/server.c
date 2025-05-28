/**
 * @file server.c
 * @author Théo AVRIL
 * @brief Serveur principal TLS pour communication sécurisée avec les clients.
 * @date 2025-05-26
 * @license MIT
 *
 * Ce fichier implémente un serveur TCP sécurisé par TLS/SSL, capable de gérer plusieurs clients
 * en parallèle grâce à des threads. Il utilise OpenSSL pour la gestion des connexions sécurisées
 * et propose une interface de communication basée sur des protocoles personnalisés.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#include "../../common/communication.h"
#include "../../common/connection.h"
#include "../../common/protocol.h"
#include "./parser.h"

#define PORT 5001
#define MAX_CLIENTS 2

/**
 * @struct ClientInfo
 * @brief Structure contenant les informations d'un client connecté.
 */
typedef struct {
    int client_socket;  /**< Descripteur de socket du client */
    struct sockaddr_in client_addr; /**< Adresse du client */
    int id; /**< Identifiant du client */
} ClientInfo;

/**
 * @struct ClientMapping
 * @brief Structure pour mapper un identifiant de client à sa communication.
 */
typedef struct {
    int id; /**< Identifiant du client */
    Communication* comm;    /**< Pointeur vers la communication du client */
} ClientMapping;

// Pointeur vers le contexte SSL
SSL_CTX* ssl_ctx = NULL;

// Tableaux pour stocker les connexions et communications des clients
Connection* server_connections[MAX_CLIENTS] = {NULL};
Communication* server_communications[MAX_CLIENTS] = {NULL};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;
int running_server = 1;

/**
 * @brief Gère le traitement des messages reçus de clients.
 * @param[in] cmd Commande reçue.
 * @param[in] param Paramètre associé à la commande.
 * @param[in] sender_id Identifiant du client émetteur.
 */
void message_handler(const char* cmd, const char* param, int sender_id) {
    pthread_t tid = pthread_self();
    printf("Thread %lu (client %d) received: Command='%s', Param='%s'\n", (unsigned long)tid, sender_id, cmd, param);
    fflush(stdout);

    if (strcmp(cmd, "CMD_X") == 0) {
        printf("Client %d says: %s\n", sender_id, param);
        
        // Création du message complet à propager
        char full_message[BUFFER_SIZE];
        snprintf(full_message, BUFFER_SIZE, "Client %d: %s", sender_id, param);
        
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            // Propagation du message aux autres clients
            if (server_communications[i] && i != sender_id) {
                server_communications[i]->comY(server_communications[i], full_message);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    } else if (strcmp(cmd, "CMD_Y") == 0) {
        printf("Command Y received from client %d: %s\n", sender_id, param);
    } else {
        printf("Unknown command received from client %d: '%s'\n", sender_id, cmd);
    }
    fflush(stdout);
}

/**
 * @brief Thread gérant la connexion et l'échange avec un client.
 * @param[in,out] arg Pointeur vers la structure ClientInfo contenant les informations du client.
 * @return NULL.
 */
void* client_handler(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_socket = client_info->client_socket;
    int client_id = client_info->id;

    printf("Client %d connected from %s:%d\n", 
           client_id, 
           inet_ntoa(client_info->client_addr.sin_addr), 
           ntohs(client_info->client_addr.sin_port));

    // Creation de la structure de connexion
    Connection* connection = Connection_create();
    connection->socket_fd = client_socket;
    connection->connected = 1;
    
    // *** TLS Handshake ***
    // Initialisation de la communication SSL/TLS
    SSL* ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        ERR_print_errors_fp(stderr);
        close(client_socket);
        free(client_info);
        return NULL;
    }
    SSL_set_fd(ssl, client_socket);
    // Après avoir configuré le contexte SSL, on effectue le handshake
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_socket);
        free(client_info);
        return NULL;
    }

    // Récupération du cipher utilisé pour la connexion SSL
    const char* cipher = SSL_get_cipher(ssl);
    printf("SSL connection established with cipher: %s\n", cipher);
    // Enregistrement de la connexion SSL dans la structure Connection
    connection->ssl = ssl;


    Protocol* protocol = Protocol_create();
    
    // Association de la connexion et de la communication
    Communication* communication = Communication_create(connection, protocol);
    communication->client_id = client_id;
    communication->setMessageHandler(communication, message_handler);
    
    pthread_mutex_lock(&clients_mutex);
    server_connections[client_id] = connection;
    server_communications[client_id] = communication;
    pthread_mutex_unlock(&clients_mutex);
    
    // Début du thread de communication
    communication->run(communication);
    
    printf("Sending welcome message to client %d\n", client_id);
    communication->comX(communication, "Welcome to the server!");
    
    // Boucle d'attente pour recevoir des messages du client
    while (connection->connected && running_server) {
        sleep(1);
    }
    
    // Fermeture de la connexion et nettoyage des ressources
    pthread_mutex_lock(&clients_mutex);
    printf("Cleaning up resources for client %d\n", client_id);
    communication->stop(communication);
    Communication_destroy(communication);
    Protocol_destroy(protocol);
    Connection_destroy(connection);
    server_connections[client_id] = NULL;
    server_communications[client_id] = NULL;
    pthread_mutex_unlock(&clients_mutex);
    
    printf("Client %d disconnected\n", client_id);
    free(client_info);
    return NULL;
}

/**
 * @brief Handler pour le signal SIGINT afin d'arrêter proprement le serveur.
 * @param[in] signal Numéro du signal reçu.
 */
void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nServer shutting down...\n");
    running_server = 0;
    if (server_socket > 0) {
        close(server_socket);
    }
    
    // Arrêt de tous les clients connectés
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server_communications[i]) {
            server_communications[i]->stop(server_communications[i]);
            Communication_destroy(server_communications[i]);
            server_communications[i] = NULL;
        }
        if (server_connections[i]) {
            Connection_destroy(server_connections[i]);
            server_connections[i] = NULL;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // Libération des ressources SSL
    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
    }

    exit(EXIT_SUCCESS);
}

/**
 * @brief Génère de nouvelles clés SSL pour le serveur si elles n'existent pas.
 * @param[in] cert_path Chemin du fichier de certificat à créer.
 * @param[in] key_path Chemin du fichier de clé privée à créer.
 * @return 1 si la génération a réussi, 0 sinon.
 */
int generate_server_keys(const char* cert_path, const char* key_path) {
    printf("Generating new SSL keys...\n");
    
    // Génération de la clé privée RSA
    RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    if (!rsa) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    
    // Création du fichier de clé privée
    FILE *private_key_file = fopen(key_path, "wb");
    if (!private_key_file) {
        perror("Failed to open private key file");
        fprintf(stderr, "Keypass: %s\n", key_path);
        RSA_free(rsa);
        return 0;
    }
    
    // Écriture de la clé privée dans le fichier
    if (!PEM_write_RSAPrivateKey(private_key_file, rsa, NULL, NULL, 0, NULL, NULL)) {
        ERR_print_errors_fp(stderr);
        fclose(private_key_file);
        RSA_free(rsa);
        return 0;
    }
    fclose(private_key_file);
    
    // Création du certificat auto-signé
    X509 *x509 = X509_new();
    if (!x509) {
        ERR_print_errors_fp(stderr);
        RSA_free(rsa);
        return 0;
    }
    
    // Sélection de la version du certificat
    X509_set_version(x509, 2); // X509v3
    
    // Sélection de l'algorithme de signature
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    
    // Sélection de la période de validité du certificat
    X509_gmtime_adj(X509_get_notBefore(x509), 0); // Valide depuis maintenant
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L); // Valide pour 1 an
    
    // Ajout de la clé publique au certificat
    EVP_PKEY *pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, rsa);
    X509_set_pubkey(x509, pkey);
    
    // Définition des informations du sujet du certificat
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"FR", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"Orion", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"Hermes", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    
    // Signature du certificat avec la clé privée
    if (!X509_sign(x509, pkey, EVP_sha256())) {
        ERR_print_errors_fp(stderr);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return 0;
    }
    
    // Création du fichier de certificat
    FILE *cert_file = fopen(cert_path, "wb");
    if (!cert_file) {
        perror("Failed to open certificate file");
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return 0;
    }
    
    // Écriture du certificat dans le fichier
    if (!PEM_write_X509(cert_file, x509)) {
        ERR_print_errors_fp(stderr);
        fclose(cert_file);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return 0;
    }
    
    fclose(cert_file);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    
    printf("New SSL keys generated successfully\n");
    return 1;
}

/**
 * @brief Point d'entrée du serveur. Initialise le contexte SSL, configure le socket, et gère les connexions entrantes.
 * @return Code de sortie, 0 si succès, sinon code d'erreur.
 */
int main() {
    load_env_file("./src/.ini");
    // Chargement des variables d'environnement
    const char *cert_path = get_env_value("CERT_PATH");
    const char *key_path = get_env_value("KEY_PATH");
    if (!cert_path || !key_path) {
        fprintf(stderr, "Missing CERT_PATH or KEY_PATH environment variables.\n");
        exit(EXIT_FAILURE);
    }

    // Initialisation du contexte SSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Génération des clés SSL si elles n'existent pas
    if (access(cert_path, F_OK) == -1) {
        if (!generate_server_keys(cert_path, key_path)) {
            fprintf(stderr, "Failed to generate SSL keys.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("SSL keys already exist, using existing files.\n");
    }

    signal(SIGINT, signal_sigint_handler);
    struct sockaddr_in server_address;
    
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // Création du contexte SSL
    const SSL_METHOD *method = TLS_server_method();
    ssl_ctx = SSL_CTX_new(method);
    if (!ssl_ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Chargement du certificat et de la clé privée
    if (SSL_CTX_use_certificate_file(ssl_ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    
    // Configuration de l'adresse du serveur
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    // Liaison du socket à l'adresse et au port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    // Mise en écoute du socket
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\nWaiting for client connections...\n", PORT);
    fflush(stdout);
    
    // Boucle principale pour accepter les connexions des clients
    while (running_server) {
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        if (!client_info) {
            perror("Memory allocation failed");
            continue;
        }
        
        // Acceptation d'une nouvelle connexion client
        socklen_t client_addr_len = sizeof(client_info->client_addr);
        client_info->client_socket = accept(server_socket, (struct sockaddr*)&client_info->client_addr, &client_addr_len);
        if (client_info->client_socket == -1) {
            if (errno == EINTR) {
                if (!running_server) {
                    free(client_info);
                    break;
                }
                continue;
            }
            perror("Accept failed");
            free(client_info);
            continue;
        }
        
        // Vérification de la limite de clients
        pthread_mutex_lock(&clients_mutex);
        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_connections[i] == NULL) {
                slot = i;
                break;
            }
        }
        
        // Si le serveur est plein, on refuse la connexion
        if (slot == -1) {
            pthread_mutex_unlock(&clients_mutex);
            printf("Server full, rejecting connection\n");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
        
        // Enregistrement de l'identifiant du client
        client_info->id = slot;
        pthread_mutex_unlock(&clients_mutex);
        
        // Création du thread pour gérer le client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, client_info) != 0) {
            perror("Thread creation failed");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
        pthread_detach(client_thread);
    }
    
    close(server_socket);
    return 0;
}
