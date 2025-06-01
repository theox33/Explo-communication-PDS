/**
 * @file Client.java
 * @brief Android client for secure messaging with SSL/TLS server
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

package com.example.server_client;
import android.content.Context;
import android.util.Log;

/**
 * @brief Main client class for managing secure server communication
 * 
 * This class provides a high-level interface for Android applications to
 * connect to and communicate with a secure messaging server. It handles
 * SSL/TLS connections, message routing, and callback-based message reception.
 */
public class Client {
    /** @brief Tag for Android logging */
    private static final String TAG = "Client";
    
    /** @brief Default server IP address to connect to */
    private static final String SERVER_IP = "192.168.1.2"; // Update to your server IP
    
    /** @brief Default server port */
    private static final int PORT = 5001;

    /** @brief Network connection handler */
    private Connection connection;
    
    /** @brief Message protocol handler */
    private Protocol protocol;
    
    /** @brief High-level communication manager */
    private Communication communication;
    
    /** @brief Initialization status flag */
    private boolean initialized = false;

    /**
     * @brief Interface for handling incoming messages from the server
     * 
     * Implementations of this interface will receive callbacks when
     * messages are received from the server or other clients.
     */
    public interface MessageListener {
        /**
         * @brief Called when a message is received
         * @param message The message content
         * @param senderId Identifier of the message sender
         */
        void onMessageReceived(String message, String senderId);
    }

    /** @brief Current message listener for callbacks */
    private MessageListener messageListener;

    /** @brief Client ID assigned by the server (-1 if not assigned) */
    private int clientId = -1;

    /**
     * @brief Constructor for the Client class
     * 
     * Initializes all communication components and sets up message handling.
     * The client is ready to connect after construction but connection must
     * be initiated explicitly.
     * 
     * @param context Android application context for SSL certificate management
     * 
     * @note The context is required for accessing Android's file system
     *       and asset management for SSL certificates
     * @note Connection is not established automatically - call connectToServer()
     */
    public Client(Context context) {
        connection = new Connection(context);
        protocol = new Protocol();
        communication = new Communication(connection, protocol);

        communication.setMessageHandler(new Communication.MessageHandler() {
            @Override
            public void onMessageReceived(String cmd, String param) {
                Log.d(TAG, "Received from server: " + param);

                if (cmd.equals(Protocol.CMD_Y)) {
                    // Extract sender info from message format "Client X: message"
                    int colonPos = param.indexOf(": ");
                    if (colonPos > 0) {
                        String senderInfo = param.substring(0, colonPos);
                        String actualMessage = param.substring(colonPos + 2);

                        // If sender info is in format "Client X"
                        if (senderInfo.startsWith("Client ")) {
                            String senderId = senderInfo.substring(7); // Get the number part

                            if (messageListener != null) {
                                messageListener.onMessageReceived(actualMessage, senderId);
                            }
                        } else {
                            // Default case if format doesn't match
                            if (messageListener != null) {
                                messageListener.onMessageReceived(param, "Server");
                            }
                        }
                    } else {
                        // Message doesn't contain sender info
                        if (messageListener != null) {
                            messageListener.onMessageReceived(param, "Server");
                        }
                    }
                }
            }
        });

        initialized = true;
    }

    /**
     * @brief Set the message listener for incoming messages
     * 
     * Registers a callback interface that will be invoked when messages
     * are received from the server or other clients.
     * 
     * @param listener The MessageListener implementation to receive callbacks
     * 
     * @note Only one listener can be active at a time
     * @note Pass null to disable message callbacks
     */
    public void setMessageListener(MessageListener listener) {
        this.messageListener = listener;
    }

    /**
     * @brief Connect to the messaging server
     * 
     * Establishes an SSL/TLS connection to the server and starts the
     * communication thread for message reception. Sends an initial
     * test message upon successful connection.
     * 
     * @return boolean True if connection was successful, false otherwise
     * 
     * @note This method should be called from a background thread
     * @note The communication thread is started automatically on success
     * @note Connection parameters (IP/port) are defined as class constants
     */
    public boolean connectToServer() {
        if (!initialized) {
            Log.e(TAG, "Client not properly initialized");
            return false;
        }

        Log.d(TAG, "Connecting to " + SERVER_IP + ":" + PORT);
        boolean connected = connection.connect(SERVER_IP, PORT);

        if (connected) {
            Log.d(TAG, "Connected to server");
            communication.run();

            // Send initial test message
            sendMessage("Test message from Android client");
        } else {
            Log.e(TAG, "Failed to connect to server");
        }

        return connected;
    }

    /**
     * @brief Send a message to the server
     * 
     * Sends a text message to the server using the CMD_X protocol command.
     * The message will be broadcast to all connected clients by the server.
     * 
     * @param message The text message to send
     * @return boolean True if message was sent successfully, false otherwise
     * 
     * @note Requires an active connection to the server
     * @note Messages are sent asynchronously
     * @note The message will be received by all other connected clients
     */
    public boolean sendMessage(String message) {
        if (!initialized || !connection.isConnected()) {
            Log.e(TAG, "Cannot send message - not connected");
            return false;
        }

        Log.d(TAG, "Sending: " + message);
        communication.comX(message);
        return true;
    }

    /**
     * @brief Disconnect from the server
     * 
     * Gracefully closes the connection to the server, stops the communication
     * thread, and cleans up all network resources.
     * 
     * @note This method is safe to call multiple times
     * @note All pending messages may be lost
     * @note The client can be reconnected after disconnection
     */
    public void disconnect() {
        if (communication != null && communication.isRunning()) {
            communication.stop();
        }

        if (connection != null) {
            connection.close();
        }

        Log.d(TAG, "Disconnected from server");
    }

    /**
     * @brief Check if the client is currently connected to the server
     * 
     * @return boolean True if connected, false otherwise
     * 
     * @note This checks both the connection object state and socket status
     * @note Connection state can change asynchronously due to network issues
     */
    public boolean isConnected() {
        return connection != null && connection.isConnected();
    }
}