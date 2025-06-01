package com.example.server_client;
import android.util.Log;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * @class Communication
 * @brief Gère l'envoi et la réception de messages entre le client et le serveur via une connexion.
 */
public class Communication {
    private static final String TAG = "Communication";
    private static final int BUFFER_SIZE = 1024; // Taille du buffer de réception

    /**
     * @interface MessageHandler
     * @brief Interface pour traiter les messages reçus.
     */
    public interface MessageHandler {
        /**
         * @brief Appelé lorsqu'un message est reçu.
         * @param cmd La commande du message.
         * @param param Le paramètre du message.
         */
        void onMessageReceived(String cmd, String param);
    }

    private Connection connection; // Gère la connexion réseau
    private Protocol protocol; // Gère l'encodage/décodage des messages
    private MessageHandler messageHandler; // Gestionnaire de messages reçus
    private Thread communicationThread; // Thread dédié à la réception des messages
    private AtomicBoolean running = new AtomicBoolean(false); // Indique si le thread tourne

    /**
     * @brief Constructeur de Communication.
     * @param connection La connexion réseau à utiliser.
     * @param protocol Le protocole de communication.
     */
    public Communication(Connection connection, Protocol protocol) {
        this.connection = connection;
        this.protocol = protocol;
    }

    /**
     * @brief Définit le gestionnaire de messages.
     * @param handler L'instance du gestionnaire.
     */
    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler;
    }

    /**
     * @brief Envoie un message de type X au serveur.
     * @param param Le paramètre à envoyer.
     */
    public void comX(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_X, param);
        connection.write(message, message.length);
    }

    /**
     * @brief Envoie un message de type Y au serveur.
     * @param param Le paramètre à envoyer.
     */
    public void comY(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_Y, param);
        connection.write(message, message.length);
    }

    /**
     * @brief Démarre le thread de communication pour recevoir les messages du serveur.
     */
    public void run() {
        if (running.get()) {
            return; // Déjà en cours d'exécution
        }

        running.set(true);

        communicationThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Communication thread started");
                byte[] buffer = new byte[BUFFER_SIZE];
                int consecutiveEmptyReads = 0; // Compte les lectures vides consécutives
                final int maxConsecutiveEmptyReads = 5; // Limite avant de vérifier la connexion

                while (running.get() && connection.isConnected()) {
                    Arrays.fill(buffer, (byte)0); // Vide le buffer

                    int bytesRead = connection.read(buffer, BUFFER_SIZE - 1);

                    if (bytesRead > 0) {
                        consecutiveEmptyReads = 0;

                        // Conversion en chaîne de caractères
                        String rawMessage = new String(buffer, 0, bytesRead);
                        Log.d(TAG, "Raw data received: '" + rawMessage + "'");

                        // Décodage du message
                        String[] parts = protocol.decodeMessage(rawMessage);

                        // Appel du gestionnaire de message
                        if (messageHandler != null) {
                            messageHandler.onMessageReceived(parts[0], parts[1]);
                        }
                    } else if (bytesRead == 0) {
                        // Pas de données disponibles (timeout)
                        consecutiveEmptyReads++;

                        if (!connection.isConnected()) {
                            Log.d(TAG, "Connection closed by peer (normal)");
                            break;
                        }

                        // Vérification des lectures vides consécutives
                        if (consecutiveEmptyReads > maxConsecutiveEmptyReads) {
                            Log.d(TAG, "Multiple empty reads, checking connection");
                            if (!connection.isConnected()) {
                                Log.d(TAG, "Connection appears broken");
                                break;
                            }
                            consecutiveEmptyReads = 0;
                        }
                    } else {
                        // Erreur de lecture
                        Log.e(TAG, "Read error, terminating communication thread");
                        break;
                    }

                    // Petite pause pour éviter de surcharger le CPU
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
     * @brief Arrête le thread de communication.
     */
    public void stop() {
        running.set(false);
        if (communicationThread != null) {
            communicationThread.interrupt();
            try {
                communicationThread.join(1000); // Attend jusqu'à 1 seconde
            } catch (InterruptedException e) {
                Log.w(TAG, "Interrupted while waiting for thread to join");
            }
            communicationThread = null;
        }
    }

    /**
     * @brief Indique si le thread de communication est en cours d'exécution.
     * @return true si le thread tourne, false sinon.
     */
    public boolean isRunning() {
        return running.get();
    }
}