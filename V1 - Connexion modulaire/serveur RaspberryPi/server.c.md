# Analyse détaillée du fichier `server.c`
## 1. Objectif du fichier
Le fichier `server.c` implémente un serveur TCP multi-clients qui:
- Écoute sur le port 5001 pour les connexions entrantes
- Gère plusieurs clients simultanément via un système de threads
- Distribue les messages reçus à tous les clients connectés
- Assure une terminaison propre lors de l'arrêt du serveur
## 2. Inclusions et définitions
```c
#include <stdio.h>          // Fonctions d'entrée/sortie standard
#include <stdlib.h>         // Fonctions standard comme malloc/free
#include <string.h>         // Manipulation de chaînes (strcmp, strcpy)
#include <unistd.h>         // Fonctions POSIX (close, sleep)
#include <pthread.h>        // Support multi-threading
#include <signal.h>         // Gestion des signaux (SIGINT)
#include <arpa/inet.h>      // Fonctions réseau (inet_ntoa, htons)
#include <fcntl.h>          // Opérations sur les descripteurs de fichiers
#include <errno.h>          // Codes d'erreur système (EINTR)
```
Ces bibliothèques standard fournissent toutes les primitives nécessaires pour créer un serveur réseau:
- `pthread.h` est essentiel pour créer un thread par client
- `arpa/inet.h` fournit les structures et fonctions pour la communication TCP/IP
- `signal.h` permet de gérer l'arrêt propre quand l'utilisateur appuie sur Ctrl+C
```c
#define PORT 5001           // Port d'écoute du serveur
#define MAX_CLIENTS 2      // Nombre maximum de clients simultanés
```
Ces constantes définissent:
- Le port utilisé par le serveur (5001) - doit être supérieur à 1024 pour être utilisé sans privilèges root
- La limite de clients simultanés (2) - limite les ressources système utilisées
## 3. Structure des données client
```c
typedef struct {
    int client_socket;           // Descripteur de socket pour ce client
    struct sockaddr_in client_addr;  // Structure contenant IP et port du client
    int id;                      // Identifiant unique dans le tableau des clients
} ClientInfo;
```
Cette structure encapsule toutes les informations nécessaires pour identifier et interagir avec un client:
- `client_socket`: Descripteur de fichier représentant la connexion socket TCP
- `client_addr`: Stocke l'adresse IP et le port du client pour l'identification et le logging
- `id`: Index unique du client dans les tableaux de gestion (permet d'identifier rapidement le client)
## 4. Variables globales
```c
Connection* server_connections[MAX_CLIENTS] = {NULL};       // Tableau des connexions actives
Communication* server_communications[MAX_CLIENTS] = {NULL}; // Tableau des gestionnaires de communication
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex pour protéger les tableaux partagés
int server_socket;               // Socket principal d'écoute
int running_server = 1;          // Indicateur d'état du serveur
```
Ces variables globales sont essentielles pour la coordination entre les threads:
- Les tableaux `server_connections` et `server_communications` stockent les objets de connexion pour chaque client. L'initialisation à `NULL` indique qu'aucun client n'est connecté initialement.
- `clients_mutex` est crucial pour éviter les conflits d'accès aux tableaux partagés lorsque plusieurs threads (clients) tentent de les modifier simultanément.
- `running_server` agit comme un flag global permettant d'arrêter proprement tous les threads lors de la fermeture du serveur.
## 5. Gestionnaire de messages
```c
void message_handler(const char* cmd, const char* param) {
    pthread_t tid = pthread_self();  // Obtient l'ID du thread courant
    
    // Affiche les détails du message reçu pour le débogage
    printf("Thread %lu received: Command='%s', Param='%s'\n", 
           (unsigned long)tid, cmd, param);
    fflush(stdout);  // Force l'affichage immédiat sans mise en buffer
```
Cette première partie de la fonction:
- Récupère l'identifiant unique du thread actuel avec `pthread_self()`
- Affiche les détails du message reçu pour faciliter le débogage
- Utilise `fflush(stdout)` pour s'assurer que les messages de débogage s'affichent immédiatement, même si la sortie standard est mise en buffer
```c
    if (strcmp(cmd, "CMD_X") == 0) {
        printf("Client %d says: %s\n", tid, param);
        
        // Diffuse le message à tous les clients connectés
        pthread_mutex_lock(&clients_mutex);  // Verrouille l'accès aux tableaux partagés
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_communications[i]) {
                server_communications[i]->comY(server_communications[i], param);
            }
        }
        pthread_mutex_unlock(&clients_mutex);  // Déverrouille après modification
```
Cette section traite spécifiquement les messages de type `CMD_X`:
- Compare la commande reçue avec `strcmp` pour identifier le type de message
- Verrouille le mutex avant d'accéder aux tableaux partagés, empêchant les autres threads de les modifier simultanément
- Parcourt tous les clients connectés et retransmet le message via la méthode `comY`
- Déverrouille le mutex une fois la diffusion terminée
```c
    } else if (strcmp(cmd, "CMD_Y") == 0) {
        printf("Command Y received: %s\n", param);
    } else {
        printf("Unknown command received: '%s'\n", cmd);
    }
    fflush(stdout);
}
```
Cette dernière partie:
- Gère différemment les messages de type `CMD_Y` (simple affichage)
- Détecte les commandes inconnues, permettant d'identifier les messages malformés ou les erreurs de protocole
- Force l'affichage des logs avant de terminer la fonction
## 6. Gestionnaire de client
```c
void* client_handler(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_socket = client_info->client_socket;
    int client_id = client_info->id;
    
    printf("Client %d connected from %s:%d\n", 
           client_id, 
           inet_ntoa(client_info->client_addr.sin_addr),  // Convertit l'adresse IP en notation pointée
           ntohs(client_info->client_addr.sin_port));     // Convertit le port de l'ordre réseau à l'ordre hôte
```
Le gestionnaire de client commence par:
- Convertir l'argument générique en structure `ClientInfo` spécifique
- Extraire le socket et l'identifiant du client pour un accès plus facile
- Afficher les informations de connexion, y compris l'adresse IP (convertie en format lisible via `inet_ntoa`) et le port (converti en ordre hôte via `ntohs`)
```c
    // Création des objets de communication pour ce client
    Connection* connection = Connection_create();
    connection->socket_fd = client_socket;
    connection->connected = 1;
    
    Protocol* protocol = Protocol_create();
    
    Communication* communication = Communication_create(connection, protocol);
    communication->setMessageHandler(communication, message_handler);
```
Cette section initialise les objets nécessaires à la communication:
- Crée un objet `Connection` qui encapsule le socket client et son état
- Crée un objet `Protocol` qui définit le format des messages échangés
- Crée un objet `Communication` qui intègre la connexion et le protocole
- Configure le gestionnaire de messages qui sera appelé à chaque réception de message
```c
    // Enregistrement des références dans les tableaux partagés
    pthread_mutex_lock(&clients_mutex);
    server_connections[client_id] = connection;
    server_communications[client_id] = communication;
    pthread_mutex_unlock(&clients_mutex);
    
    // Démarrage du thread de communication
    communication->run(communication);
    
    // Envoi d'un message de bienvenue
    printf("Sending welcome message to client %d\n", client_id);
    communication->comX(communication, "Welcome to the server!");
```
Cette partie:
- Protège l'accès aux tableaux partagés avec le mutex avant de les modifier
- Enregistre les objets de connexion créés dans les tableaux globaux
- Lance le thread de réception des messages via `communication->run()`
- Envoie un message de bienvenue au client nouvellement connecté
```c
    // Surveille l'état de la connexion
    while (connection->connected && running_server) {
        sleep(1);  // Vérification périodique toutes les secondes
    }
```
Cette boucle de surveillance:
- Vérifie périodiquement si la connexion est toujours active ou si le serveur doit s'arrêter
- Utilise `sleep(1)` pour éviter d'utiliser 100% du CPU dans une boucle d'attente active
- Se termine lorsque la connexion est fermée ou que le serveur est arrêté
```c
    // Nettoyage des ressources
    pthread_mutex_lock(&clients_mutex);
    printf("Cleaning up resources for client %d\n", client_id);
    communication->stop(communication);          // Arrête le thread de communication
    Communication_destroy(communication);        // Libère l'objet de communication
    Protocol_destroy(protocol);                  // Libère l'objet de protocole
    Connection_destroy(connection);              // Ferme la connexion et libère l'objet
    server_connections[client_id] = NULL;        // Marque l'emplacement comme disponible
    server_communications[client_id] = NULL;
    pthread_mutex_unlock(&clients_mutex);
    
    printf("Client %d disconnected\n", client_id);
    free(client_info);                           // Libère la structure d'information client
    return NULL;
}
```
Cette section finale:
- Protège l'accès aux tableaux partagés avant de les modifier
- Effectue un nettoyage ordonné de toutes les ressources allouées
- Libère tous les objets dans l'ordre inverse de leur création
- Marque l'emplacement client comme disponible en réinitialisant les entrées à NULL
- Libère la structure d'information client et termine le thread
## 7. Gestionnaire de signal (Ctrl+C)
```c
void signal_sigint_handler(int signal) {
    fprintf(stdout, "\nServer shutting down...\n");
    running_server = 0;  // Signale à toutes les boucles de s'arrêter
    // Ferme le socket serveur pour interrompre accept()
    if (server_socket > 0) {
        close(server_socket);
    }
```
Le gestionnaire de signal SIGINT:
- Est déclenché lorsque l'utilisateur appuie sur Ctrl+C
- Indique que le serveur s'arrête en mettant `running_server` à 0
- Ferme le socket principal pour débloquer la fonction `accept()` qui serait en attente
```c
    // Nettoie toutes les connexions clients
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
    exit(EXIT_SUCCESS);  // Termine le programme proprement
}
```
Cette partie:
- Verrouille le mutex pour protéger l'accès aux tableaux partagés
- Parcourt tous les clients connectés et libère méthodiquement leurs ressources
- Déverrouille le mutex une fois les nettoyages terminés
- Termine le programme proprement avec `exit(EXIT_SUCCESS)`
## 8. Fonction principale (main)
```c
int main() {
    signal(SIGINT, signal_sigint_handler);  // Enregistre le gestionnaire de signal
    struct sockaddr_in server_address;
```
La fonction `main()` commence par:
- Configurer le gestionnaire de signal pour intercepter Ctrl+C (SIGINT)
- Déclarer la structure d'adresse du serveur
```c
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
```
Cette section:
- Crée un socket avec `socket()` en spécifiant:
  - `AF_INET`: Famille d'adresses IPv4
  - `SOCK_STREAM`: Type de socket orienté connexion (TCP)
  - `0`: Protocole par défaut pour ce type de socket
- Vérifie si la création a réussi et sort avec un message d'erreur si ce n'est pas le cas
```c
    // Configuration de l'option SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
```
Cette partie:
- Configure l'option `SO_REUSEADDR` qui permet de réutiliser l'adresse locale
- Cette option est importante pour éviter les erreurs "Address already in use" lors du redémarrage rapide du serveur
- Vérifie si la configuration a réussi
```c
    // Configuration de l'adresse du serveur
    memset(&server_address, 0, sizeof(server_address));  // Initialise la structure à zéro
    server_address.sin_family = AF_INET;                 // Famille d'adresses IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;         // Écoute sur toutes les interfaces réseau
    server_address.sin_port = htons(PORT);               // Port d'écoute (converti en ordre réseau)
```
Cette section:
- Initialise la structure d'adresse à zéro avec `memset()` pour éviter des valeurs non définies
- Configure les champs de la structure:
  - `sin_family`: Type d'adresse (IPv4)
  - `sin_addr.s_addr`: Adresse IP (INADDR_ANY = toutes les interfaces)
  - `sin_port`: Port d'écoute, converti en ordre réseau avec `htons()`
```c
    // Association du socket à l'adresse et au port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    // Mise en écoute du socket
    if (listen(server_socket, 5) == -1) {  // File d'attente de 5 connexions maximum
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
```
Cette partie:
- Associe le socket à l'adresse et au port spécifiés avec `bind()`
- Met le socket en mode écoute avec `listen()`:
  - Le paramètre 5 indique la taille de la file d'attente des connexions entrantes
  - Les connections supplémentaires seront refusées si la file est pleine
- Vérifie les erreurs à chaque étape et sort proprement en cas d'échec

```c
    // Boucle principale - acceptation et gestion des clients
    while (running_server) {
        // Allocation de la structure d'information client
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        if (!client_info) {
            perror("Memory allocation failed");
            continue;  // Continue la boucle si l'allocation échoue
        }
```
La boucle principale:
- S'exécute tant que `running_server` est vrai (modifié par le gestionnaire de signal)
- Alloue dynamiquement une structure `ClientInfo` pour chaque nouvelle connexion
- Vérifie si l'allocation a réussi
```c
        // Acceptation d'une nouvelle connexion
        socklen_t client_addr_len = sizeof(client_info->client_addr);
        client_info->client_socket = accept(server_socket, 
                                           (struct sockaddr*)&client_info->client_addr, 
                                           &client_addr_len);
```
Cette section:
- Appelle `accept()` qui bloque jusqu'à ce qu'une nouvelle connexion arrive
- Stocke le descripteur de socket retourné dans la structure `client_info`
- Récupère automatiquement l'adresse et le port du client dans `client_info->client_addr`
```c
        if (client_info->client_socket == -1) {
            if (errno == EINTR) {
                // Interrompu par un signal, vérifie si le serveur doit continuer
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
```
Cette gestion d'erreur:
- Vérifie si `accept()` a échoué (retourne -1)
- Traite spécialement l'erreur `EINTR` qui indique une interruption par un signal
- Vérifie l'état de `running_server` pour décider s'il faut sortir de la boucle
- Libère la mémoire allouée et continue en cas d'erreur
```c
        // Recherche d'un emplacement libre pour le client
        pthread_mutex_lock(&clients_mutex);
        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_connections[i] == NULL) {
                slot = i;
                break;
            }
        }
```
Cette partie:
- Verrouille le mutex pour accéder aux tableaux partagés de manière sécurisée
- Parcourt le tableau des connexions pour trouver un emplacement libre (NULL)
- S'arrête dès qu'un emplacement est trouvé et stocke son index
```c
        if (slot == -1) {
            // Aucun emplacement disponible - serveur plein
            pthread_mutex_unlock(&clients_mutex);
            printf("Server full, rejecting connection\n");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
```
Cette section:
- Vérifie si un emplacement a été trouvé (`slot == -1` signifie serveur plein)
- Déverrouille le mutex avant de continuer
- Rejette la connexion en fermant le socket et libérant la mémoire
- Continue la boucle pour accepter de nouvelles connexions
```c
        client_info->id = slot;
        pthread_mutex_unlock(&clients_mutex);
```
Cette ligne:
- Assigne l'index de l'emplacement comme identifiant du client
- Déverrouille le mutex puisque l'accès aux tableaux partagés est terminé
```c
        // Création d'un thread pour gérer ce client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, client_info) != 0) {
            perror("Thread creation failed");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }
        
        // Détachement du thread pour libération automatique des ressources
        pthread_detach(client_thread);
    }
```
La dernière partie:
- Crée un nouveau thread pour gérer ce client spécifique
- Passe la structure `client_info` comme argument au thread
- Détache le thread pour qu'il se nettoie automatiquement à la fin
- Continue la boucle pour accepter la connexion suivante
```c
    close(server_socket);
    return 0;
}
```
Enfin, après la boucle:
- Ferme le socket principal du serveur
- Retourne 0 pour indiquer une terminaison réussie
