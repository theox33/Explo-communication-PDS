/**
 * @file server.c
 * @brief Multi-threaded SSL/TLS server implementation
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
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
 
 #include "../../common/communication.h"
 #include "../../common/connection.h"
 #include "../../common/protocol.h"
 #include "./parser.h"
 
 /** @brief Default server port */
 #define PORT 5001
 
 /** @brief Maximum number of concurrent clients */
 #define MAX_CLIENTS 2
 
 /**
  * @brief Structure containing client connection information
  */
 typedef struct {
     int client_socket;              /**< Client socket file descriptor */
     struct sockaddr_in client_addr; /**< Client address information */
     int id;                         /**< Client ID (slot index) */
 } ClientInfo;
 
 /** @brief Global TLS context pointer for SSL operations */
 SSL_CTX* ssl_ctx = NULL;
 
 /** @brief Array of active server connections */
 Connection* server_connections[MAX_CLIENTS] = {NULL};
 
 /** @brief Array of active server communications */
 Communication* server_communications[MAX_CLIENTS] = {NULL};
 
 /** @brief Mutex for thread-safe access to client arrays */
 pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 /** @brief Main server socket file descriptor */
 int server_socket;
 
 /** @brief Server running flag (1=running, 0=shutting down) */
 int running_server = 1;
 
 /**
  * @brief Message handler callback for processing received commands
  * 
  * Processes incoming commands from clients and handles message broadcasting.
  * Currently supports CMD_X for message broadcasting and CMD_Y for acknowledgment.
  * 
  * @param cmd The received command string
  * @param param The received parameter string
  * 
  * @note CMD_X messages are broadcast to all connected clients
  * @note CMD_Y messages are logged but not processed further
  * @note Unknown commands are logged with a warning message
  */
 void message_handler(const char* cmd, const char* param) {
     pthread_t tid = pthread_self();
     printf("Thread %lu received: Command='%s', Param='%s'\n", 
            (unsigned long)tid, cmd, param);
     fflush(stdout);
 
     if (strcmp(cmd, "CMD_X") == 0) {
         printf("Client %lu says: %s\n", (unsigned long)tid, param);
         pthread_mutex_lock(&clients_mutex);
         for (int i = 0; i < MAX_CLIENTS; i++) {
             if (server_communications[i]) {
                 server_communications[i]->comY(server_communications[i], param);
             }
         }
         pthread_mutex_unlock(&clients_mutex);
     } else if (strcmp(cmd, "CMD_Y") == 0) {
         printf("Command Y received: %s\n", param);
     } else {
         printf("Unknown command received: '%s'\n", cmd);
     }
     fflush(stdout);
 }
 
 /**
  * @brief Client handler thread function
  * 
  * Handles a single client connection in a dedicated thread. Performs SSL/TLS
  * handshake, creates communication objects, and manages the client session
  * until disconnection.
  * 
  * @param arg Pointer to ClientInfo structure (cast from void*)
  * @return void* Always returns NULL
  * 
  * @note Performs SSL_accept() for TLS handshake
  * @note Logs the cipher suite used for the connection
  * @note Sends a welcome message to newly connected clients
  * @note Cleans up all resources when client disconnects
  */
 void* client_handler(void* arg) {
     ClientInfo* client_info = (ClientInfo*)arg;
     int client_socket = client_info->client_socket;
     int client_id = client_info->id;
 
     printf("Client %d connected from %s:%d\n", 
            client_id, 
            inet_ntoa(client_info->client_addr.sin_addr), 
            ntohs(client_info->client_addr.sin_port));
 
     // Create connection object
     Connection* connection = Connection_create();
     connection->socket_fd = client_socket;
     connection->connected = 1;
     
     // *** TLS Handshake ***
     // Create an SSL object using the global TLS context.
     SSL* ssl = SSL_new(ssl_ctx);
     if (!ssl) {
         ERR_print_errors_fp(stderr);
         close(client_socket);
         free(client_info);
         return NULL;
     }
     SSL_set_fd(ssl, client_socket);
     // After successful SSL_accept in client_handler:
     if (SSL_accept(ssl) <= 0) {
         ERR_print_errors_fp(stderr);
         SSL_free(ssl);
         close(client_socket);
         free(client_info);
         return NULL;
     }
 
     // Retrieve and log the cipher suite used for the connection
     const char* cipher = SSL_get_cipher(ssl);
     printf("SSL connection established with cipher: %s\n", cipher);
     // Save the SSL pointer in the connection so that our read/write functions use it.
     connection->ssl = ssl;
     // ********************************
 
     // Create protocol object
     Protocol* protocol = Protocol_create();
     
     // Create communication object
     Communication* communication = Communication_create(connection, protocol);
     communication->setMessageHandler(communication, message_handler);
     
     pthread_mutex_lock(&clients_mutex);
     server_connections[client_id] = connection;
     server_communications[client_id] = communication;
     pthread_mutex_unlock(&clients_mutex);
     
     // Start the communication
     communication->run(communication);
     
     printf("Sending welcome message to client %d\n", client_id);
     communication->comX(communication, "Welcome to the server!");
     
     while (connection->connected && running_server) {
         sleep(1);
     }
     
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
  * @brief Signal handler for graceful server shutdown
  * 
  * Handles SIGINT (Ctrl+C) signal to perform graceful server shutdown.
  * Closes all client connections, frees SSL context, and terminates the server.
  * 
  * @param signal Signal number (expected to be SIGINT)
  * 
  * @note Sets running_server flag to 0 to stop main loop
  * @note Closes the main server socket
  * @note Stops and destroys all active client communications and connections
  * @note Frees the global SSL context
  * @note Calls exit() to terminate the process
  */
 void signal_sigint_handler(int signal) {
     fprintf(stdout, "\nServer shutting down...\n");
     running_server = 0;
     if (server_socket > 0) {
         close(server_socket);
     }
     
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
 
     // Free the global SSL context
     if (ssl_ctx) {
         SSL_CTX_free(ssl_ctx);
     }
 
     exit(EXIT_SUCCESS);
 }
 
 /**
  * @brief Main server function
  * 
  * Initializes the SSL/TLS server, loads configuration, creates the listening
  * socket, and enters the main accept loop to handle incoming client connections.
  * 
  * @return int Exit status (0 for success, non-zero for failure)
  * 
  * @note Loads certificate and key paths from .ini configuration file
  * @note Initializes OpenSSL library and creates SSL context
  * @note Configures socket options including SO_REUSEADDR
  * @note Accepts up to MAX_CLIENTS concurrent connections
  * @note Creates a new thread for each client connection
  * @note Handles server shutdown gracefully on SIGINT signal
  */
 int main() {
     load_env_file(".ini");
     // Load environment variables
     const char *cert_path = get_env_value("CERT_PATH");
     const char *key_path = get_env_value("KEY_PATH");
     if (!cert_path && !key_path) {
         fprintf(stderr, "Missing CERT_PATH and KEY_PATH environment variables.\n");
         exit(EXIT_FAILURE);
     }
 
     signal(SIGINT, signal_sigint_handler);
     struct sockaddr_in server_address;
     
     // Create server socket
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
     
     // Initialize OpenSSL and set up TLS context
     SSL_library_init();
     SSL_load_error_strings();
     OpenSSL_add_all_algorithms();
     const SSL_METHOD *method = TLS_server_method();
     ssl_ctx = SSL_CTX_new(method);
     if (!ssl_ctx) {
         ERR_print_errors_fp(stderr);
         exit(EXIT_FAILURE);
     }
     if (SSL_CTX_use_certificate_file(ssl_ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
         ERR_print_errors_fp(stderr);
         exit(EXIT_FAILURE);
     }
     if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
         ERR_print_errors_fp(stderr);
         exit(EXIT_FAILURE);
     }
     
     memset(&server_address, 0, sizeof(server_address));
     server_address.sin_family = AF_INET;
     server_address.sin_addr.s_addr = INADDR_ANY;
     server_address.sin_port = htons(PORT);
     
     if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
         perror("Bind failed");
         close(server_socket);
         exit(EXIT_FAILURE);
     }
     
     if (listen(server_socket, 5) == -1) {
         perror("Listen failed");
         close(server_socket);
         exit(EXIT_FAILURE);
     }
     
     printf("Server started on port %d\nWaiting for client connections...\n", PORT);
     fflush(stdout);
     
     while (running_server) {
         ClientInfo* client_info = malloc(sizeof(ClientInfo));
         if (!client_info) {
             perror("Memory allocation failed");
             continue;
         }
         
         socklen_t client_addr_len = sizeof(client_info->client_addr);
         client_info->client_socket = accept(server_socket, 
                                            (struct sockaddr*)&client_info->client_addr, 
                                            &client_addr_len);
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
         
         pthread_mutex_lock(&clients_mutex);
         int slot = -1;
         for (int i = 0; i < MAX_CLIENTS; i++) {
             if (server_connections[i] == NULL) {
                 slot = i;
                 break;
             }
         }
         
         if (slot == -1) {
             pthread_mutex_unlock(&clients_mutex);
             printf("Server full, rejecting connection\n");
             close(client_info->client_socket);
             free(client_info);
             continue;
         }
         
         client_info->id = slot;
         pthread_mutex_unlock(&clients_mutex);
         
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