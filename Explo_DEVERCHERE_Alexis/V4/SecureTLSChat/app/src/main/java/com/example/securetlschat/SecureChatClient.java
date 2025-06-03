/**
 * @file SecureChatClient.java
 * @brief TLS-secured chat client implementation for Android
 * @version 4.0
 * @author Alexis DEVERCHERE
 *
 * @section description Description
 * Handles all network communication with the chat server using TLS encryption.
 * Manages connection lifecycle and message exchange in background threads.
 *
 * @section features Features
 * - TLS 1.2/1.3 secure communication
 * - Asynchronous message handling
 * - Connection state management
 * - Thread-safe operations
 */

package com.example.securetlschat;

import android.content.Context;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.net.ssl.SSLSocket;

public class SecureChatClient {
    private static final String TAG = "SecureChatClient";

    private final Context context;
    private final String serverHost;
    private final int serverPort;
    private SSLSocket socket;
    private PrintWriter out;
    private BufferedReader in;
    private final ExecutorService executor = Executors.newCachedThreadPool();
    private MessageListener messageListener;
    private ConnectionListener connectionListener;
    private volatile boolean connected = false;

    /**
     * @interface MessageListener
     * @brief Callback interface for message events
     */
    public interface MessageListener {
        void onMessageReceived(String message);
        void onError(String error);
    }

    /**
     * @interface ConnectionListener
     * @brief Callback interface for connection events
     */
    public interface ConnectionListener {
        void onConnected();
        void onDisconnected();
        void onConnectionError(String error);
    }

    /**
     * @brief Constructor for SecureChatClient
     * @param context Android context
     * @param host Server hostname/IP
     * @param port Server port
     */
    public SecureChatClient(Context context, String host, int port) {
        this.context = context;
        this.serverHost = host;
        this.serverPort = port;
    }

    /**
     * @brief Set message event listener
     * @param listener MessageListener implementation
     */
    public void setMessageListener(MessageListener listener) {
        this.messageListener = listener;
    }

    /**
     * @brief Set connection event listener
     * @param listener ConnectionListener implementation
     */
    public void setConnectionListener(ConnectionListener listener) {
        this.connectionListener = listener;
    }

    /**
     * @brief Establish connection to chat server
     */
    public void connect() {
        executor.execute(() -> {
            try {
                Log.d(TAG, "Connecting to " + serverHost + ":" + serverPort);

                TLSSocketFactory tlsFactory = new TLSSocketFactory(context, serverHost, serverPort);
                socket = tlsFactory.createSocket();

                out = new PrintWriter(socket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                connected = true;
                Log.d(TAG, "Connected successfully");

                if (connectionListener != null) {
                    connectionListener.onConnected();
                }

                startMessageReader();

            } catch (IOException e) {
                Log.e(TAG, "Connection error", e);
                if (connectionListener != null) {
                    connectionListener.onConnectionError(e.getMessage());
                }
            }
        });
    }

    /**
     * @brief Start background thread for reading incoming messages
     */
    private void startMessageReader() {
        executor.execute(() -> {
            try {
                String message;
                while (connected && (message = in.readLine()) != null) {
                    Log.d(TAG, "Received: " + message);
                    if (messageListener != null) {
                        final String finalMessage = message;
                        messageListener.onMessageReceived(finalMessage);
                    }
                }
            } catch (IOException e) {
                if (connected) {
                    Log.e(TAG, "Error reading message", e);
                    if (messageListener != null) {
                        messageListener.onError("Connection lost: " + e.getMessage());
                    }
                }
            } finally {
                disconnect();
            }
        });
    }

    /**
     * @brief Send message to server
     * @param message Message to send
     */
    public void sendMessage(String message) {
        if (!connected || out == null) {
            Log.e(TAG, "Not connected");
            if (messageListener != null) {
                messageListener.onError("Not connected to server");
            }
            return;
        }

        executor.execute(() -> {
            out.println(message);
            Log.d(TAG, "Sent: " + message);
        });
    }

    /**
     * @brief Disconnect from server
     */
    public void disconnect() {
        connected = false;

        try {
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
            if (out != null) {
                out.close();
            }
            if (in != null) {
                in.close();
            }

            if (connectionListener != null) {
                connectionListener.onDisconnected();
            }

        } catch (IOException e) {
            Log.e(TAG, "Error during disconnect", e);
        }
    }

    /**
     * @brief Check connection status
     * @return boolean True if connected
     */
    public boolean isConnected() {
        return connected && socket != null && !socket.isClosed();
    }

    /**
     * @brief Clean up resources
     */
    public void destroy() {
        disconnect();
        executor.shutdown();
    }
}