package com.example.server_client;
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
        void onMessageReceived(String message);
    }

    private MessageListener messageListener;

    public Client() {
        connection = new Connection();
        protocol = new Protocol();
        communication = new Communication(connection, protocol);

        communication.setMessageHandler(new Communication.MessageHandler() {
            @Override
            public void onMessageReceived(String cmd, String param) {
                Log.d(TAG, "Received from server: " + param);
                if (messageListener != null) {
                    messageListener.onMessageReceived(param);
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