/**
 * @file protocol.h
 * @brief Message protocol for command encoding/decoding
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #ifndef PROTOCOL_H
 #define PROTOCOL_H
 
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 
 /** @brief Maximum size for command strings */
 #define MAX_CMD_SIZE 32
 
 /** @brief Maximum size for parameter strings */
 #define MAX_PARAM_SIZE 1024
 
 /**
  * @brief Protocol structure for message encoding and decoding
  * 
  * This structure defines a simple protocol for encoding and decoding
  * messages in the format "COMMAND|PARAMETER". It provides predefined
  * command constants and function pointers for message processing.
  */
 typedef struct Protocol {
     const char* cmdX;    /**< Predefined command X identifier */
     const char* cmdY;    /**< Predefined command Y identifier */
     
     // Method pointers for object-oriented interface
     /**
      * @brief Encode a command and parameter into a message string
      * @param protocol Pointer to the Protocol object
      * @param cmd Command string to encode
      * @param param Parameter string to encode
      * @param out_buffer Output buffer to store the encoded message
      */
     void (*encodeMessage)(struct Protocol*, const char*, const char*, char*);
     
     /**
      * @brief Decode a message string into command and parameter components
      * @param protocol Pointer to the Protocol object
      * @param message Input message string to decode
      * @param cmd Output buffer for the decoded command
      * @param param Output buffer for the decoded parameter
      */
     void (*decodeMessage)(struct Protocol*, const char*, char*, char*);
 } Protocol;
 
 /**
  * @brief Create a new Protocol object
  * 
  * Allocates memory for a new Protocol structure and initializes it with
  * predefined command constants and method pointers.
  * 
  * @return Protocol* Pointer to the newly created Protocol object, or NULL on failure
  * 
  * @note The caller is responsible for calling Protocol_destroy() to free resources
  * @note Command constants are initialized to "CMD_X" and "CMD_Y"
  */
 Protocol* Protocol_create();
 
 /**
  * @brief Destroy a Protocol object and free its resources
  * 
  * Frees the memory allocated for the Protocol structure.
  * 
  * @param protocol Pointer to the Protocol object to destroy
  * 
  * @note It's safe to pass NULL to this function
  */
 void Protocol_destroy(Protocol* protocol);
 
 #endif // PROTOCOL_H