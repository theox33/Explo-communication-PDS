package com.example.server_client;
import android.content.Context;
import android.util.Log;

public class Client {
    private static final String TAG = "Client";
    private static final String SERVER_IP = "192.168.1.2"; // Update to your server IP
    private static final int PORT = 5001;

    private Connection connection;
    private Protocol protocol;
    private Communication communication;
    private boolean initialized = false;

    public interface MessageListener {
        void onMessageReceived(String message, String senderId);
    }

    private MessageListener messageListener;

    private int clientId = -1;

    // Update constructor to accept a Context parameter
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

    public void setMessageListener(MessageListener listener) {
        this.messageListener = listener;
    }

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

    public boolean sendMessage(String message) {
        if (!initialized || !connection.isConnected()) {
            Log.e(TAG, "Cannot send message - not connected");
            return false;
        }

        Log.d(TAG, "Sending: " + message);
        communication.comX(message);
        return true;
    }

    public void disconnect() {
        if (communication != null && communication.isRunning()) {
            communication.stop();
        }

        if (connection != null) {
            connection.close();
        }

        Log.d(TAG, "Disconnected from server");
    }

    public boolean isConnected() {
        return connection != null && connection.isConnected();
    }
}