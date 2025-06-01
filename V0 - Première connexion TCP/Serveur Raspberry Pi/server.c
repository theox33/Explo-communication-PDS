/**
 * @file server.c
 * @brief Serveur TCP simple en C pour communication avec client Android
 * @details Ce serveur implémente un serveur TCP qui écoute sur le port 8080
 *          et peut recevoir des messages de clients Android via socket TCP.
 *          Le serveur traite une connexion à la fois et renvoie une réponse
 *          de confirmation pour chaque message reçu.
 * @author Alexis DEVERCHERE
 * @date 2025
 * @version 0.1
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 
 /**
  * @def PORT
  * @brief Port d'écoute du serveur TCP
  * @details Le serveur écoute sur ce port pour les connexions entrantes
  */
 #define PORT 8080
 
 /**
  * @def BUFFER_SIZE
  * @brief Taille maximale du buffer de réception
  * @details Taille en octets du buffer utilisé pour recevoir les messages clients
  */
 #define BUFFER_SIZE 1024
 
 /**
  * @brief Point d'entrée principal du serveur TCP
  * @details Cette fonction initialise un serveur TCP qui :
  *          - Crée un socket serveur
  *          - Configure les options du socket
  *          - Se lie au port spécifié
  *          - Écoute les connexions entrantes
  *          - Accepte et traite les connexions clients
  *          - Renvoie une réponse de confirmation
  * 
  * @return int Code de retour (0 en cas de succès, EXIT_FAILURE en cas d'erreur)
  * 
  * @note Le serveur fonctionne en boucle infinie et traite les connexions séquentiellement
  * @warning Le serveur n'implémente aucune sécurité ou authentification
  */
 int main() {
     /**
      * @var server_fd
      * @brief Descripteur de fichier du socket serveur
      */
     int server_fd, new_socket;
     
     /**
      * @var address
      * @brief Structure contenant les informations d'adresse du serveur
      * @details Contient la famille d'adresse (AF_INET), l'adresse IP et le port
      */
     struct sockaddr_in address;
     
     /**
      * @var opt
      * @brief Option pour la réutilisation de l'adresse socket
      * @details Permet de réutiliser immédiatement l'adresse après fermeture
      */
     int opt = 1;
     
     /**
      * @var addrlen
      * @brief Taille de la structure d'adresse
      */
     int addrlen = sizeof(address);
     
     /**
      * @var buffer
      * @brief Buffer de réception des messages clients
      * @details Initialisé à zéro, utilisé pour stocker les messages reçus
      */
     char buffer[BUFFER_SIZE] = {0};
     
     /**
      * @var response
      * @brief Message de réponse standard envoyé aux clients
      */
     char *response = "Message reçu par le serveur C";
     
     /**
      * @brief Création du socket TCP
      * @details Crée un socket IPv4 (AF_INET) de type flux (SOCK_STREAM) avec protocole TCP
      */
     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
         perror("Échec de création du socket");
         exit(EXIT_FAILURE);
     }
     
     /**
      * @brief Configuration des options du socket
      * @details Active SO_REUSEADDR pour permettre la réutilisation immédiate de l'adresse
      * @note Évite l'erreur "Address already in use" lors du redémarrage du serveur
      */
     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
         perror("Échec setsockopt");
         exit(EXIT_FAILURE);
     }
     
     /**
      * @brief Configuration de la structure d'adresse serveur
      * @details - sin_family : IPv4 (AF_INET)
      *          - sin_addr : Toutes les interfaces (INADDR_ANY)
      *          - sin_port : Port d'écoute converti en ordre réseau
      */
     address.sin_family = AF_INET;
     address.sin_addr.s_addr = INADDR_ANY;
     address.sin_port = htons(PORT);
     
     /**
      * @brief Liaison du socket à l'adresse et au port
      * @details Associe le socket serveur à l'adresse IP et au port spécifiés
      */
     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
         perror("Échec du bind");
         exit(EXIT_FAILURE);
     }
     
     /**
      * @brief Mise en écoute du socket
      * @details Configure le socket pour accepter les connexions entrantes
      * @param server_fd Descripteur du socket serveur
      * @param 3 Nombre maximum de connexions en attente dans la queue
      */
     if (listen(server_fd, 3) < 0) {
         perror("Échec du listen");
         exit(EXIT_FAILURE);
     }
     
     printf("Serveur C en écoute sur le port %d...\n", PORT);
     
     /**
      * @brief Boucle principale du serveur
      * @details Traite les connexions clients en continu :
      *          1. Accepte une nouvelle connexion
      *          2. Lit le message du client
      *          3. Envoie une réponse
      *          4. Ferme la connexion
      *          5. Nettoie le buffer pour la prochaine connexion
      */
     while (1) {
         /**
          * @brief Acceptation d'une nouvelle connexion client
          * @details Bloque jusqu'à ce qu'un client se connecte
          * @return new_socket Descripteur de la nouvelle connexion client
          */
         if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
             perror("Échec de l'accept");
             exit(EXIT_FAILURE);
         }
         
         /**
          * @brief Affichage des informations de connexion
          * @details Affiche l'adresse IP et le port du client connecté
          */
         printf("Connexion acceptée depuis %s:%d\n",
                inet_ntoa(address.sin_addr), ntohs(address.sin_port));
         
         /**
          * @brief Lecture du message du client
          * @details Lit les données envoyées par le client dans le buffer
          * @param new_socket Socket de la connexion client
          * @param buffer Buffer de réception
          * @param BUFFER_SIZE Taille maximale à lire
          * @return valread Nombre d'octets lus
          */
         int valread = read(new_socket, buffer, BUFFER_SIZE);
         if (valread > 0) {
             printf("Message reçu: %s\n", buffer);
             
             /**
              * @brief Envoi de la réponse au client
              * @details Envoie le message de confirmation au client
              * @param new_socket Socket de la connexion client
              * @param response Message de réponse
              * @param strlen(response) Taille du message
              * @param 0 Flags (aucun flag spécial)
              */
             send(new_socket, response, strlen(response), 0);
             printf("Réponse envoyée\n");
         }
         
         /**
          * @brief Fermeture de la connexion client
          * @details Ferme proprement la connexion avec le client
          */
         close(new_socket);
         
         /**
          * @brief Réinitialisation du buffer
          * @details Remet à zéro le buffer pour la prochaine connexion
          */
         memset(buffer, 0, BUFFER_SIZE);
     }
     
     /**
      * @brief Fermeture du socket serveur
      * @details Code jamais atteint dans cette implémentation (boucle infinie)
      * @note Ce code pourrait être utile si un mécanisme d'arrêt était implémenté
      */
     close(server_fd);
     return 0;
 }