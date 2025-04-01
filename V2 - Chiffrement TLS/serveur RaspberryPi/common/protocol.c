// protocol.c
#include "protocol.h"

static void Protocol_encodeMessage(Protocol* protocol, const char* cmd, const char* param, char* out_buffer) {
    // Format: "CMD|PARAM\0"
    int offset = 0;
    
    // Copy command
    while (*cmd) {
        out_buffer[offset++] = *cmd++;
    }
    
    // Add separator
    out_buffer[offset++] = '|';
    
    // Copy parameter
    while (*param) {
        out_buffer[offset++] = *param++;
    }
    
    // Ensure null termination
    out_buffer[offset] = '\0';
    
    printf("Encoded message: '%s'\n", out_buffer);
}

static void Protocol_decodeMessage(Protocol* protocol, const char* message, char* cmd, char* param) {
    // Parse "CMD|PARAM" format
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
        
        // Assign method pointers
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