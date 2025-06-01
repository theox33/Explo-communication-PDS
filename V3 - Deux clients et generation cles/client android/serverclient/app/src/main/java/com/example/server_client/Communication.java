/**
 * @file Communication.java
 * @brief High-level communication interface for Android messaging
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

package com.example.server_client;
import android.util.Log;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * @brief High-level communication manager for networked messaging
 * 
 * This class combines Connection and Protocol objects to provide a complete
 * messaging solution. It manages a background thread for continuous message
 * reception and provides methods for sending different types of commands.
 */
public class Communication {
    /** @brief Tag for Android logging */
    private static final String TAG = "Communication";
    
    /** @brief Buffer size for message reception */
    private static final int BUFFER_SIZE = 1024;

    /**
     * @brief Interface for handling received messages
     * 
     * Implementations receive callbacks when messages are decoded
     * from the network stream.
     */
    public interface MessageHandler {
        /**
         * @brief Called when a message is received and decoded
         * @param cmd The decoded command string
         * @param param The decoded parameter string
         */
        void onMessageReceived(String cmd, String param);
    }

    /** @brief Network connection object */
    private Connection connection;
    
    /** @brief Message protocol handler */
    private Protocol protocol;
    
    /** @brief Current message handler for callbacks */
    private MessageHandler messageHandler;
    
    /** @brief Background thread for message reception */
    private Thread communicationThread;
    
    /** @brief Thread-safe running flag */
    private AtomicBoolean running = new AtomicBoolean(false);

    /**
     * @brief Constructor for Communication class
     * 
     * Initializes the communication manager with the provided connection
     * and protocol objects. The communication thread is not started until
     * run() is called.
     * 
     * @param connection Network connection object
     * @param protocol Message protocol handler
     * 
     * @note The connection should be established before starting communication
     * @note The protocol object defines the message encoding/decoding format
     */
    public Communication(Connection connection, Protocol protocol) {
        this.connection = connection;
        this.protocol = protocol;
    }

    /**
     * @brief Set the message handler for incoming messages
     * 
     * Registers a callback interface that will be invoked when messages
     * are received and successfully decoded.
     * 
     * @param handler The MessageHandler implementation to receive callbacks
     * 
     * @note Only one handler can be active at a time
     * @note The handler is called from the communication thread context
     */
    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler;
    }

    /**
     * @brief Send a CMD_X message with parameter
     * 
     * Encodes and sends a CMD_X command message with the specified parameter.
     * This command type is typically used for client-to-server messaging.
     * 
     * @param param Parameter string to send with the command
     * 
     * @note The message is encoded using the protocol's encodeMessage method
     * @note Requires an active connection to succeed
     */
    public void comX(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_X, param);
        connection.write(message, message.length);
    }

    /**
     * @brief Send a CMD_Y message with parameter
     * 
     * Encodes and sends a CMD_Y command message with the specified parameter.
     * This command type is typically used for server-to-client messaging.
     * 
     * @param param Parameter string to send with the command
     * 
     * @note The message is encoded using the protocol's encodeMessage method
     * @note Requires an active connection to succeed
     * @note There appears to be a duplicate write call in the original code
     */
    public void comY(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_Y, param);
        connection.write(message, message.length);
        connection.write(message, message.length); // Note: Duplicate write in original
    }

    /**
     * @brief Start the communication thread for message reception
     * 
     * Creates and starts a background thread that continuously monitors
     * the connection for incoming messages. Messages are decoded and
     * passed to the registered message handler.
     * 
     * @note If already running, this method returns without creating a new thread
     * @note The thread includes connection health monitoring
     * @note Uses a 50ms delay between read attempts to prevent CPU hogging
     */
    public void run() {
        if (running.get()) {
            return; // Already running
        }

        running.set(true);

        communicationThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Communication thread started");
                byte[] buffer = new byte[BUFFER_SIZE];
                int consecutiveEmptyReads = 0;
                final int maxConsecutiveEmptyReads = 5;

                while (running.get() && connection.isConnected()) {
                    Arrays.fill(buffer, (byte)0); // Clear buffer

                    int bytesRead = connection.read(buffer, BUFFER_SIZE - 1);

                    if (bytesRead > 0) {
                        consecutiveEmptyReads = 0;

                        // Ensure null termination and convert to string
                        String rawMessage = new String(buffer, 0, bytesRead);
                        Log.d(TAG, "Raw data received: '" + rawMessage + "'");

                        // Process the message
                        String[] parts = protocol.decodeMessage(rawMessage);

                        // Call the message handler if set
                        if (messageHandler != null) {
                            messageHandler.onMessageReceived(parts[0], parts[1]);
                        }
                    } else if (bytesRead == 0) {
                        // No data available
                        consecutiveEmptyReads++;

                        // If connection lost
                        if (!connection.isConnected()) {
                            Log.d(TAG, "Connection closed by peer (normal)");
                            break;
                        }

                        // Connection check after multiple empty reads
                        if (consecutiveEmptyReads > maxConsecutiveEmptyReads) {
                            Log.d(TAG, "Multiple empty reads, checking connection");
                            if (!connection.isConnected()) {
                                Log.d(TAG, "Connection appears broken");
                                break;
                            }
                            consecutiveEmptyReads = 0;
                        }
                    } else {
                        // Error occurred
                        Log.e(TAG, "Read error, terminating communication thread");
                        break;
                    }

                    // Small delay to prevent CPU hogging
                    try {
                        Thread.sleep(50);
                    } catch (InterruptedException e) {
                        break;
                    }
                }

                Log.d(TAG, "Communication thread exiting");
                running.set(false);
            }
        });

        communicationThread.start();
    }

    /**
     * @brief Stop the communication thread
     * 
     * Signals the communication thread to stop and waits for it to terminate.
     * This method blocks until the thread has completely stopped.
     * 
     * @note Safe to call multiple times
     * @note Waits up to 1 second for thread termination
     * @note If interrupted during join, a warning is logged
     */
    public void stop() {
        running.set(false);
        if (communicationThread != null) {
            communicationThread.interrupt();
            try {
                communicationThread.join(1000); // Wait up to 1 second for thread to finish
            } catch (InterruptedException e) {
                Log.w(TAG, "Interrupted while waiting for thread to join");
            }
            communicationThread = null;
        }
    }

    /**
     * @brief Check if the communication thread is currently running
     * 
     * @return boolean True if the communication thread is active, false otherwise
     * 
     * @note This reflects the thread state, not necessarily the connection state
     * @note Thread-safe using AtomicBoolean
     */
    public boolean isRunning() {
        return running.get();
    }
}