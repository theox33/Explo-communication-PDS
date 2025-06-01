#include "protocol.h"

static void Protocol_encodeMessage(Protocol* protocol, const char* cmd, const char* param, char* out_buffer) {
    // Format : "CMD|PARAM\0"
    int offset = 0;
    
    // Copier la commande
    while (*cmd) {
        out_buffer[offset++] = *cmd++;
    }
    
    // Ajouter le séparateur
    out_buffer[offset++] = '|';
    
    // Copier le paramètre
    while (*param) {
        out_buffer[offset++] = *param++;
    }
    
    // S'assurer de la terminaison nulle
    out_buffer[offset] = '\0';
    
    printf("Message encodé : '%s'\n", out_buffer);
}

static void Protocol_decodeMessage(Protocol* protocol, const char* message, char* cmd, char* param) {
    // Analyser le format "CMD|PARAM"
    char* delimiter = strchr(message, '|');
    if (delimiter) {
        int cmd_length = delimiter - message;
        strncpy(cmd, message, cmd_length);
        cmd[cmd_length] = '\0';
        
        strcpy(param, delimiter + 1);
    } else {
        strcpy(cmd, message);
        param[0] = '\0';
    }
}

Protocol* Protocol_create() {
    Protocol* protocol = (Protocol*)malloc(sizeof(Protocol));
    if (protocol) {
        protocol->cmdX = "CMD_X";
        protocol->cmdY = "CMD_Y";
        
        // Assigner les pointeurs de méthode
        protocol->encodeMessage = Protocol_encodeMessage;
        protocol->decodeMessage = Protocol_decodeMessage;
    }
    return protocol;
}

void Protocol_destroy(Protocol* protocol) {
    if (protocol) {
        free(protocol);
    }
}