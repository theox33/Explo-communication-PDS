/**
 * @file Connection.java
 * @brief Activité Android pour gérer la connexion à un serveur TCP et l'échange de messages Protobuf.
 *
 * Cette activité permet à l'utilisateur de se connecter à un serveur, d'envoyer et de recevoir des messages
 * sérialisés avec Protobuf, et d'afficher les échanges dans une interface graphique. Les messages sont chiffrés
 * avec une transformation XOR simple avant l'envoi et déchiffrés à la réception.
 *
 * Fonctionnalités principales :
 * - Connexion et déconnexion au serveur via un bouton.
 * - Saisie et envoi de messages texte.
 * - Réception et affichage des messages du serveur.
 * - Utilisation de threads pour la communication réseau afin de ne pas bloquer l'UI.
 * - Chiffrement/déchiffrement XOR des messages.
 *
 * @author Théo AVRIL
 * @license MIT
 */

package com.example.client_android;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

/**
 * @class Connection
 * @brief Activité Android pour gérer la connexion à un serveur et l'échange de messages.
 *
 * Cette classe gère l'interface utilisateur et la logique de communication réseau :
 * - Gestion de la connexion/déconnexion au serveur.
 * - Envoi de messages au serveur.
 * - Réception asynchrone des messages du serveur.
 * - Affichage des messages dans une zone dédiée.
 * - Chiffrement/déchiffrement simple des messages.
 */
public class Connection extends AppCompatActivity {

    /** Bouton pour se connecter/déconnecter du serveur */
    private Button connectButton, sendButton;
    /** Champ de saisie du message */
    private EditText messageInput;
    /** Zone d'affichage des messages */
    private TextView messagesDisplay;
    /** ScrollView pour faire défiler les messages */
    private ScrollView scrollView;

    /** Socket de connexion au serveur */
    private Socket socket;
    /** Flux de sortie pour envoyer des messages */
    private OutputStream out;
    /** Thread pour la réception des messages */
    private Thread receiveThread;
    /** Indique si la connexion est active */
    private boolean isConnected = false;

    /** Adresse IP du serveur */
    private final String SERVER_IP = "192.168.1.24";
    /** Port du serveur */
    private final int SERVER_PORT = 12345;

    /**
     * Méthode appelée à la création de l'activité.
     * Initialise l'interface et les listeners des boutons.
     * @param savedInstanceState Etat sauvegardé de l'activité
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Récupération des vues depuis le layout
        connectButton = findViewById(R.id.connect_button);
        sendButton = findViewById(R.id.send_button);
        messageInput = findViewById(R.id.message_input);
        messagesDisplay = findViewById(R.id.messages_display);
        scrollView = findViewById(R.id.scroll_view);

        // Désactive le bouton d'envoi tant qu'on n'est pas connecté
        sendButton.setEnabled(false);

        // Gestion du clic sur le bouton de connexion/déconnexion
        connectButton.setOnClickListener(v -> {
            if (!isConnected) {
                connectToServer();
            } else {
                disconnectFromServer();
            }
        });

        // Gestion du clic sur le bouton d'envoi de message
        sendButton.setOnClickListener(v -> {
            String messageText = messageInput.getText().toString();
            if (!messageText.isEmpty()) {
                postMessage(messageText);
                messageInput.setText("");
                appendMessage("Ares: " + messageText);
            }
        });
    }

    /**
     * Tente de se connecter au serveur et démarre le thread de réception.
     */
    private void connectToServer() {
        new Thread(() -> {
            try {
                // Connexion au serveur
                socket = new Socket(SERVER_IP, SERVER_PORT);
                out = socket.getOutputStream();
                isConnected = true;

                // Mise à jour de l'UI après connexion
                runOnUiThread(() -> {
                    connectButton.setText("Disconnect");
                    sendButton.setEnabled(true);
                    appendMessage("[Connected to Hermes]");
                });

                // Thread pour recevoir les messages du serveur
                receiveThread = new Thread(() -> {
                    try {
                        InputStream input = socket.getInputStream();
                        byte[] buffer = new byte[1024];
                        while (isConnected) {
                            int bytesRead = input.read(buffer);
                            if (bytesRead == -1) break;

                            // Extraction des données reçues
                            byte[] actualData = new byte[bytesRead];
                            System.arraycopy(buffer, 0, actualData, 0, bytesRead);

                            // Décodage du message protobuf et déchiffrement
                            Message.AMessage msg = Message.AMessage.parseFrom(actualData);
                            String decrypted = xorTransform(msg.getContent());

                            // Affichage du message reçu
                            runOnUiThread(() -> appendMessage("Hermes: " + decrypted));
                        }
                    } catch (Exception e) {
                        runOnUiThread(() -> appendMessage("[Reception error: " + e.getMessage() + "]"));
                        e.printStackTrace();
                    }
                });
                receiveThread.start();

            } catch (Exception e) {
                runOnUiThread(() -> appendMessage("[Connection failed: " + e.getMessage() + "]"));
                e.printStackTrace();
            }
        }).start();
    }

    /**
     * Déconnecte du serveur et arrête le thread de réception.
     */
    private void disconnectFromServer() {
        try {
            isConnected = false;
            if (socket != null) socket.close();
            if (receiveThread != null && receiveThread.isAlive()) receiveThread.interrupt();

            // Mise à jour de l'UI après déconnexion
            runOnUiThread(() -> {
                connectButton.setText("Connect");
                sendButton.setEnabled(false);
                appendMessage("[Disconnected from Hermes]");
            });

        } catch (Exception e) {
            runOnUiThread(() -> appendMessage("[Disconnection error: " + e.getMessage() + "]"));
            e.printStackTrace();
        }
    }

    /**
     * Envoie un message au serveur.
     * @param messageText Le texte du message à envoyer
     */
    private void postMessage(String messageText) {
        new Thread(() -> {
            try {
                // Création du message protobuf avec chiffrement XOR
                Message.AMessage protoMessage = Message.AMessage.newBuilder()
                        .setContent(xorTransform(messageText))
                        .build();

                // Envoi du message au serveur
                byte[] data = protoMessage.toByteArray();
                out.write(data);
                out.flush();
            } catch (Exception e) {
                runOnUiThread(() -> appendMessage("[Send error: " + e.getMessage() + "]"));
                e.printStackTrace();
            }
        }).start();
    }

    /**
     * Ajoute un message à l'affichage et fait défiler vers le bas.
     * @param message Le message à afficher
     */
    private void appendMessage(String message) {
        messagesDisplay.append(message + "\n");
        scrollView.post(() -> scrollView.fullScroll(View.FOCUS_DOWN));
    }

    /**
     * Applique une transformation XOR sur la chaîne.
     * @param input La chaîne à transformer
     * @return La chaîne transformée
     */
    private String xorTransform(String input) {
        char[] chars = input.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            chars[i] ^= 0x5A;
        }
        return new String(chars);
    }

    /**
     * Appelée lors de la destruction de l'activité.
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnectFromServer();
    }
}