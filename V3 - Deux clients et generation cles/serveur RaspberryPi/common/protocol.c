/**
 * @file protocol.c
 * @brief Implementation of message protocol for command encoding/decoding
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #include "protocol.h"

 /**
  * @brief Encode a command and parameter into a protocol message
  * 
  * Creates a formatted message string in the format "COMMAND|PARAMETER".
  * The function manually copies characters to build the message without
  * using higher-level string functions.
  * 
  * @param protocol Pointer to the Protocol object (unused in current implementation)
  * @param cmd Command string to encode
  * @param param Parameter string to encode
  * @param out_buffer Output buffer to store the encoded message
  * 
  * @note The output buffer must be large enough to hold the entire message
  * @note The encoded message is null-terminated
  * @note Debug information is printed showing the encoded message
  */
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
 
 /**
  * @brief Decode a protocol message into command and parameter components
  * 
  * Parses a message string in the format "COMMAND|PARAMETER" and extracts
  * the command and parameter into separate buffers.
  * 
  * @param protocol Pointer to the Protocol object (unused in current implementation)
  * @param message Input message string to decode
  * @param cmd Output buffer for the decoded command
  * @param param Output buffer for the decoded parameter
  * 
  * @note If no '|' separator is found, the entire message is treated as a command
  *       and the parameter is set to empty string
  * @note The output buffers must be large enough to hold the decoded strings
  * @note Both output strings are null-terminated
  */
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
 
 /**
  * @brief Create a new Protocol object
  * 
  * Allocates memory for a new Protocol structure and initializes it with
  * predefined command constants and method pointers for encoding/decoding operations.
  * 
  * @return Protocol* Pointer to the newly created Protocol object, or NULL on allocation failure
  * 
  * @note The command constants are set to "CMD_X" and "CMD_Y"
  * @note Method pointers are assigned to enable object-oriented usage
  * @note The caller must call Protocol_destroy() to free the allocated memory
  */
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
 
 /**
  * @brief Destroy a Protocol object and free its resources
  * 
  * Frees the memory allocated for the Protocol structure.
  * 
  * @param protocol Pointer to the Protocol object to destroy
  * 
  * @note It's safe to pass NULL to this function
  * @note This function only frees the Protocol structure itself,
  *       not the string constants which are static
  */
 void Protocol_destroy(Protocol* protocol) {
     if (protocol) {
         free(protocol);
     }
 }