# Analyse d'un système de communication client-serveur en C
## Table des matières
1. [Objectif du Projet](#objectif-du-projet)
2. [Architecture](#architecture)
3. [server.c](#serverc)
    - [Objectifs](#objectifs-de-serverc)
    - [Implémentation](#implémentation-de-serverc)
    - [Structures de données clés](#structures-de-données-clés)
    - [Gestion des connexions](#gestion-des-connexions)
4. [communication.c](#communicationc)
    - [Objectif](#objectif-de-communicationc)
    - [Fonctionnalités](#fonctionnalités-de-communicationc)
    - [Gestion des messages](#gestion-des-messages)
5. [connection.c](#connectionc)
    - [Objectif](#objectif-de-connectionc)
    - [Configuration TCP](#configuration-tcp)
    - [Opérations de base](#opérations-de-base)
6. [protocol.c](#protocolc)
    - [Objectif](#objectif-de-protocolc)
    - [Format des messages](#format-des-messages)
    - [Implémentation](#implémentation-du-protocol)
7. [Flux d'exécution](#flux-dexécution)
8. [Aspects techniques notables](#aspects-techniques-notables)
## Objectif du Projet
Ce projet implémente un système de communication client-serveur en langage C où :
- Le serveur peut accepter et gérer plusieurs connexions clients simultanément
- Les clients peuvent envoyer des messages au serveur
- Le serveur peut traiter ces messages et envoyer des réponses
- La communication est bidirectionnelle et asynchrone
- Le système est conçu de manière modulaire avec une séparation claire des responsabilités
## Architecture
Le projet est organisé selon une architecture modulaire avec quatre composants principaux :
1. **Server** : Point d'entrée qui gère l'écoute, l'acceptation et la distribution des connexions clients
2. **Communication** : Gère l'échange de messages haut niveau
3. **Connection** : Gère les opérations de sockets de bas niveau 
4. **Protocol** : Définit le format d'encodage et de décodage des messages
```
           ┌─────────────┐
           │   Server    │
           └─────┬───────┘
                 │ crée et gère
                 ▼
           ┌─────────────┐
           │ ClientInfo  │──┐
           └─────┬───────┘  │ thread par client
                 │ utilise  │
                 ▼          │
┌─────────────┬─────────────┬─────────────┐
│ Connection  │Communication│  Protocol   │
└─────────────┴─────────────┴─────────────┘
```
## server.c
### Objectifs de server.c
Le fichier `server.c` est le composant principal qui :
- Initialise le socket serveur sur le port 5001
- Accepte les connexions entrantes des clients
- Crée un thread dédié pour chaque client connecté
- Gère la synchronisation entre les threads clients via mutex
- Assure une terminaison propre lors de la réception d'un signal d'interruption
### Implémentation de server.c
Le code s'organise autour des éléments suivants :
1. **Initialisation du serveur**
```c
server_socket = socket(AF_INET, SOCK_STREAM, 0);
setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
listen(server_socket, 5);
```
2. **Boucle d'acceptation des clients**
```c
while (running_server) {
    ClientInfo* client_info = malloc(sizeof(ClientInfo));
    client_info->client_socket = accept(server_socket, 
                                     (struct sockaddr*)&client_info->client_addr, 
                                     &client_addr_len);
    // Trouver un slot disponible et démarrer un thread client
    pthread_create(&client_thread, NULL, client_handler, client_info);
    pthread_detach(client_thread);
}
```
### Structures de données clés
- **ClientInfo** : Stocke les informations relatives à chaque client connecté
```c
typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    int id;
} ClientInfo;
```
- **Tableaux globaux** : Maintiennent les références aux objets de communication et de connexion
```c
Connection* server_connections[MAX_CLIENTS] = {NULL};
Communication* server_communications[MAX_CLIENTS] = {NULL};
```
### Gestion des connexions
Le `client_handler` est responsable de gérer la durée de vie d'une connexion client :
1. Il crée les objets nécessaires (Connection, Protocol, Communication)
2. Il configure le gestionnaire de messages
3. Il surveille l'état de la connexion et libère les ressources lorsque le client se déconnecte
La synchronisation est assurée par le mutex `clients_mutex` pour éviter les problèmes d'accès concurrents aux tableaux de connexions.
## communication.c
### Objectif de communication.c
Ce module fournit une couche d'abstraction pour l'échange de messages structurés entre le serveur et les clients. Il :
- Encode et décode les messages en utilisant le protocole défini
- Gère la communication asynchrone via un thread dédié
- Fournit des fonctions de haut niveau pour envoyer des commandes (comX, comY)
- Détecte les déconnexions et les erreurs de communication
### Fonctionnalités de communication.c
1. **Fonctions d'envoi de messages**
```c
static void Communication_comX(Communication* comm, const char* param) {
    char buffer[BUFFER_SIZE];
    comm->protocol->encodeMessage(comm->protocol, comm->protocol->cmdX, param, buffer);
    comm->connection->write(comm->connection, buffer, strlen(buffer) + 1);
}
```
2. **Thread de communication**
```c
static void* communication_thread_function(void* arg) {
    // Boucle de lecture des messages entrants
    while (comm->running) {
        ssize_t bytes_read = comm->connection->read(...);
        // Décodage et traitement des messages
    }
}
```
### Gestion des messages
L'interface `Communication` utilise un callback `MessageHandler` pour traiter les messages reçus :
```c
typedef void (*MessageHandler)(const char* cmd, const char* param);
```
Lorsqu'un message est reçu et décodé, le gestionnaire est appelé avec les commandes et paramètres extraits :
```c
if (comm->messageHandler) {
    comm->messageHandler(cmd, param);
}
```
## connection.c
### Objectif de connection.c
Ce module gère les opérations de sockets TCP de bas niveau, notamment :
- La création et configuration des sockets
- L'établissement des connexions
- Les opérations de lecture et d'écriture sur le socket
- La gestion des erreurs de connexion
- La détection des déconnexions
### Configuration TCP
Le code configure les sockets TCP avec des mécanismes de keepalive pour détecter les connexions interrompues :
```c
int keepalive = 1;
int keepidle = 60;   // Commencer les sondes après 60 secondes d'inactivité
int keepintvl = 10;  // Envoyer une sonde toutes les 10 secondes
int keepcnt = 5;     // Déconnecter après 5 sondes échouées
setsockopt(conn->socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
setsockopt(conn->socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
```
### Opérations de base
Les opérations fondamentales comprennent :
1. **Lecture synchronisée**
```c
static ssize_t Connection_read(Connection* conn, void* buffer, size_t length) {
    pthread_mutex_lock(&conn->mutex);
    // Configuration du timeout
    result = recv(conn->socket_fd, buffer, length, 0);
    // Gestion des erreurs
    pthread_mutex_unlock(&conn->mutex);
    return result;
}
```
2. **Écriture synchronisée**
```c
static ssize_t Connection_write(Connection* conn, const void* buffer, size_t length) {
    pthread_mutex_lock(&conn->mutex);
    result = send(conn->socket_fd, buffer, length, 0);
    pthread_mutex_unlock(&conn->mutex);
    return result;
}
```
## protocol.c
### Objectif de protocol.c
Ce module définit le format des messages échangés entre le serveur et les clients. Il :
- Encode les messages avant envoi
- Décode les messages à la réception
- Définit les types de commandes disponibles (cmdX, cmdY)
### Format des messages
Le protocole utilise un format simple délimité par un caractère pipe :
```
CMD|PARAM
```
Par exemple :
- `CMD_X|Hello World` pour envoyer "Hello World" via la commande X
- `CMD_Y|Status OK` pour envoyer "Status OK" via la commande Y
### Implémentation du protocol
L'encodage transforme une commande et un paramètre en chaîne formatée :
```c
static void Protocol_encodeMessage(Protocol* protocol, const char* cmd, const char* param, char* out_buffer) {
    // Formatage : "CMD|PARAM\0"
    // ...
}
```
Le décodage extrait la commande et le paramètre d'une chaîne reçue :
```c
static void Protocol_decodeMessage(Protocol* protocol, const char* message, char* cmd, char* param) {
    // Parsing du format "CMD|PARAM"
    char* delimiter = strchr(message, '|');
    // ...
}
```
## Flux d'exécution
Le flux d'exécution typique du système est le suivant :
1. **Démarrage du serveur** :
   - Initialisation du socket serveur
   - Configuration du gestionnaire de signaux
   - Attente des connexions sur le port 5001
2. **Connexion d'un client** :
   - Acceptation de la connexion
   - Allocation d'un slot dans les tableaux server_connections et server_communications
   - Création d'un thread dédié pour gérer le client
3. **Communication** :
   - Le serveur envoie un message de bienvenue au client
   - Le client envoie un message au serveur via comX
   - Le thread de communication reçoit le message, le décode et appelle le gestionnaire
   - Le serveur peut répondre ou transmettre le message à d'autres clients
4. **Déconnexion** :
   - Détection de la fermeture de connexion (lecture retournant 0 ou keepalive échoué)
   - Libération des ressources associées au client
   - Notification de la déconnexion
## Aspects techniques notables
1. **Gestion de la concurrence** :
   - Utilisation de mutex pour protéger les ressources partagées
   - Threads détachés pour gérer chaque client indépendamment
   - Synchronisation entre les différents composants
2. **Robustesse** :
   - Détection des connexions interrompues via keepalive TCP
   - Timeouts sur les opérations de lecture pour éviter les blocages
   - Gestionnaire de signaux pour assurer une terminaison propre
3. **Modularité** :
   - Séparation claire des responsabilités entre les composants
   - Interfaces bien définies entre les modules
   - Utilisation de structures orientées objet en C (structs avec pointeurs de fonction)
Le système fournit ainsi une architecture solide pour implémenter un serveur capable de gérer plusieurs clients simultanément avec une communication bidirectionnelle efficace.