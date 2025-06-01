package com.example.server_client;
import android.content.Context;
import android.util.Log;

/**
 * @class Client
 * @brief Gère la connexion, la communication et le protocole avec le serveur.
 */
public class Client {
    private static final String TAG = "Client";
    private static final int PORT = 5001; ///< Port utilisé pour la connexion au serveur

    private Connection connection; ///< Gère la connexion réseau
    private Protocol protocol; ///< Gère l'encodage/décodage des messages
    private Communication communication; ///< Gère l'envoi/réception des messages
    private boolean initialized = false; ///< Indique si le client est initialisé

    /**
     * @interface MessageListener
     * @brief Interface pour recevoir les messages du serveur.
     */
    public interface MessageListener {
        /**
         * @brief Appelé lorsqu'un message est reçu du serveur.
         * @param message Le message reçu.
         */
        void onMessageReceived(String message);
    }

    private MessageListener messageListener; ///< Listener pour les messages reçus

    /**
     * @brief Constructeur du client.
     * @param context Le contexte Android.
     */
    public Client(Context context) {
        connection = new Connection(context);
        protocol = new Protocol();
        communication = new Communication(connection, protocol);

        // Définit le gestionnaire de messages pour la communication
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

    /**
     * @brief Définit le listener pour les messages reçus.
     * @param listener L'instance du listener.
     */
    public void setMessageListener(MessageListener listener) {
        this.messageListener = listener;
    }

    /**
     * @brief Tente de se connecter au serveur.
     * @param ip L'adresse IP du serveur entrée dans le champs de texte.
     * @return true si la connexion a réussi, false sinon.
     */
    public boolean connectToServer(String ip) {
        if (!initialized) {
            Log.e(TAG, "Client not properly initialized");
            return false;
        }

        if (ip == null || ip.isEmpty()) {
            Log.e(TAG, "Invalid IP address");
            return false;
        }

        Log.d(TAG, "Connecting to " + ip + ":" + PORT);
        boolean connected = connection.connect(ip, PORT);

        if (connected) {
            Log.d(TAG, "Connected to server");
            communication.run();

            // Envoie un message de test initial
            sendMessage("Test message from Android client");
        } else {
            Log.e(TAG, "Failed to connect to server");
        }

        return connected;
    }

    /**
     * @brief Envoie un message au serveur.
     * @param message Le message à envoyer.
     * @return true si l'envoi a réussi, false sinon.
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
     * @brief Déconnecte le client du serveur.
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
     * @brief Vérifie si le client est connecté au serveur.
     * @return true si connecté, false sinon.
     */
    public boolean isConnected() {
        return connection != null && connection.isConnected();
    }
}