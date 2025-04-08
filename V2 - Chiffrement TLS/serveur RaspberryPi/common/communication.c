// communication.c
#include "communication.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void Communication_comX(Communication* comm, const char* param) {
    char buffer[BUFFER_SIZE];
    comm->protocol->encodeMessage(comm->protocol, comm->protocol->cmdX, param, buffer);
    comm->connection->write(comm->connection, buffer, strlen(buffer) + 1);
}

static void Communication_comY(Communication* comm, const char* param) {
    char buffer[BUFFER_SIZE];
    comm->protocol->encodeMessage(comm->protocol, comm->protocol->cmdY, param, buffer);
    comm->connection->write(comm->connection, buffer, strlen(buffer) + 1);
}

static void* communication_thread_function(void* arg) {
    Communication* comm = (Communication*)arg;
    char buffer[BUFFER_SIZE];
    char cmd[MAX_CMD_SIZE];
    char param[MAX_PARAM_SIZE];
    int consecutive_empty_reads = 0;
    const int max_consecutive_empty_reads = 5;
    
    printf("Thread de communication démarré\n");
    
    while (comm->running) {
        // Effacer les tampons avant chaque lecture
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t bytes_read = comm->connection->read(comm->connection, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read > 0) {
            // Réinitialiser le compteur de lectures vides après réception de données
            consecutive_empty_reads = 0;
            
            // S'assurer de la terminaison nulle
            buffer[bytes_read] = '\0';
            printf("Données brutes reçues : '%s'\n", buffer);
            
            // Effacer les tampons de commande et de paramètre
            memset(cmd, 0, MAX_CMD_SIZE);
            memset(param, 0, MAX_PARAM_SIZE);
            
            // Traiter le message
            comm->protocol->decodeMessage(comm->protocol, buffer, cmd, param);
            
            // Appeler le gestionnaire de messages s'il est défini
            if (comm->messageHandler) {
                comm->messageHandler(cmd, param, comm->client_id);
            }
        } else if (bytes_read == 0) {
            // Aucune donnée disponible
            consecutive_empty_reads++;
            
            // Vérifier si la connexion a été marquée comme fermée pendant la lecture
            if (!comm->connection->connected) {
                printf("Connexion fermée par le pair (normal)\n");
                break;
            }
            
            // Si trop de lectures vides consécutives, vérifier la connexion
            if (consecutive_empty_reads > max_consecutive_empty_reads) {
                // Essayer une sonde de 0 octet pour tester la connexion
                char probe_buffer[1];
                ssize_t probe_result = send(comm->connection->socket_fd, probe_buffer, 0, MSG_NOSIGNAL);
                
                if (probe_result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    // La connexion est probablement rompue
                    printf("La connexion semble rompue après plusieurs lectures vides\n");
                    comm->connection->connected = 0;
                    break;
                }
                
                // Réinitialiser le compteur après vérification de la connexion
                consecutive_empty_reads = 0;
            }
        } else {
            // Une erreur réelle s'est produite lors de la lecture
            printf("Erreur de lecture, arrêt du thread de communication\n");
            break;
        }
        
        // Petit délai pour éviter une boucle occupée qui consomme trop de CPU
        usleep(50000);  // Délai de 50ms
    }
    
    printf("Fin du thread de communication\n");
    return NULL;
}

static void Communication_run(Communication* comm) {
    comm->running = 1;
    pthread_create(&comm->thread, NULL, communication_thread_function, comm);
}

static void Communication_stop(Communication* comm) {
    comm->running = 0;
    if (comm->thread) {
        pthread_join(comm->thread, NULL);
        comm->thread = 0;
    }
}

static void Communication_setMessageHandler(Communication* comm, MessageHandler handler) {
    comm->messageHandler = handler;
}

Communication* Communication_create(Connection* connection, Protocol* protocol) {
    Communication* comm = (Communication*)malloc(sizeof(Communication));
    if (comm) {
        comm->connection = connection;
        comm->protocol = protocol;
        comm->running = 0;
        comm->thread = 0;
        comm->messageHandler = NULL;
        
        // Assigner les pointeurs de méthode
        comm->comX = Communication_comX;
        comm->comY = Communication_comY;
        comm->run = Communication_run;
        comm->stop = Communication_stop;
        comm->setMessageHandler = Communication_setMessageHandler;
    }
    return comm;
}

void Communication_destroy(Communication* comm) {
    if (comm) {
        // Arrêter le thread de communication
        if (comm->running) {
            comm->stop(comm);
        }
        
        // Ne pas détruire la connexion et le protocole ici car ils peuvent être partagés
        free(comm);
    }
}