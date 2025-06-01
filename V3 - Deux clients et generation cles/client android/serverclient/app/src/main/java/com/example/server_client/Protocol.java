/**
 * @file Protocol.java
 * @brief Message protocol implementation for Android client-server communication
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

package com.example.server_client;
import android.util.Log;

/**
 * @brief Simple message protocol for encoding and decoding commands
 * 
 * This class implements a lightweight protocol for client-server communication
 * using the format "COMMAND|PARAMETER". It provides predefined command constants
 * and methods for encoding/decoding messages compatible with the C server implementation.
 */
public class Protocol {
    /** @brief Tag for Android logging */
    private static final String TAG = "Protocol";
    
    /** @brief Command constant for client-to-server messages */
    public static final String CMD_X = "CMD_X";
    
    /** @brief Command constant for server-to-client messages */
    public static final String CMD_Y = "CMD_Y";

    /**
     * @brief Encode a command and parameter into a protocol message
     * 
     * Creates a formatted message string in the format "COMMAND|PARAMETER"
     * with a null terminator to match the C server implementation.
     * 
     * @param cmd Command string to encode (typically CMD_X or CMD_Y)
     * @param param Parameter string to include with the command
     * @return byte[] Encoded message as byte array with null terminator
     * 
     * @note The null terminator is added for compatibility with C server
     * @note Debug information is logged showing the encoded message
     * @note The returned byte array can be sent directly over the network
     */
    public byte[] encodeMessage(String cmd, String param) {
        String message = cmd + "|" + param;
        Log.d(TAG, "Encoded message: " + message);
        return (message + "\0").getBytes(); // Add null terminator like C code
    }

    /**
     * @brief Decode a protocol message into command and parameter components
     * 
     * Parses a message string in the format "COMMAND|PARAMETER" and extracts
     * the command and parameter into separate strings.
     * 
     * @param message Input message string to decode
     * @return String[] Array containing [command, parameter]
     * 
     * @note If no '|' separator is found, the entire message is treated as command
     *       and parameter is set to empty string
     * @note The returned array always has exactly 2 elements: [cmd, param]
     * @note Compatible with messages from the C server implementation
     */
    public String[] decodeMessage(String message) {
        String[] parts = new String[]{"", ""};
        int separatorIndex = message.indexOf('|');

        if (separatorIndex != -1) {
            parts[0] = message.substring(0, separatorIndex); // Command
            parts[1] = message.substring(separatorIndex + 1); // Parameter
        } else {
            parts[0] = message;
        }

        return parts;
    }
}