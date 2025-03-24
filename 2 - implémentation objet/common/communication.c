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
    
    printf("Communication thread started\n");
    
    while (comm->running) {
        // Clear buffers before each read
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t bytes_read = comm->connection->read(comm->connection, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read > 0) {
            // Reset empty read counter on successful data reception
            consecutive_empty_reads = 0;
            
            // Ensure null termination
            buffer[bytes_read] = '\0';
            printf("Raw data received: '%s'\n", buffer);
            
            // Clear command and parameter buffers
            memset(cmd, 0, MAX_CMD_SIZE);
            memset(param, 0, MAX_PARAM_SIZE);
            
            // Process the message
            comm->protocol->decodeMessage(comm->protocol, buffer, cmd, param);
            
            // Call the message handler if set
            if (comm->messageHandler) {
                comm->messageHandler(cmd, param);
            }
        } else if (bytes_read == 0) {
            // No data available
            consecutive_empty_reads++;
            
            // Check if connection was marked as closed during read
            if (!comm->connection->connected) {
                printf("Connection closed by peer (normal)\n");
                break;
            }
            
            // If we've had too many consecutive empty reads, check connection
            if (consecutive_empty_reads > max_consecutive_empty_reads) {
                // Try a zero-byte probe to test connection
                char probe_buffer[1];
                ssize_t probe_result = send(comm->connection->socket_fd, probe_buffer, 0, MSG_NOSIGNAL);
                
                if (probe_result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    // Connection is likely broken
                    printf("Connection appears broken after multiple empty reads\n");
                    comm->connection->connected = 0;
                    break;
                }
                
                // Reset counter after connection check
                consecutive_empty_reads = 0;
            }
        } else {
            // Actual error occurred in read
            printf("Read error, terminating communication thread\n");
            break;
        }
        
        // Small delay to prevent CPU hogging on busy loop
        usleep(50000);  // 50ms delay
    }
    
    printf("Communication thread exiting\n");
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
        
        // Assign method pointers
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
        // Stop the communication thread
        if (comm->running) {
            comm->stop(comm);
        }
        
        // Don't destroy connection and protocol here as they might be shared
        free(comm);
    }
}