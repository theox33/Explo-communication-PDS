package com.example.server_client;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Activité principale de l'application client.
 * Gère l'interface utilisateur et la communication avec le serveur.
 */
public class MainActivity extends AppCompatActivity {
    private TextView ipInput; // Champ de saisie de l'adresse IP
    private Client client; // Instance du client réseau
    private EditText messageInput; // Champ de saisie du message
    private Button sendButton; // Bouton d'envoi
    private Button connectButton; // Bouton de connexion/déconnexion
    private TextView messagesDisplay; // Affichage des messages
    private ScrollView scrollView; // Pour faire défiler les messages
    private Handler mainHandler; // Handler pour exécuter du code sur le thread principal

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialisation des composants de l'UI
        ipInput = findViewById(R.id.ip_input);
        messageInput = findViewById(R.id.message_input);
        sendButton = findViewById(R.id.send_button);
        connectButton = findViewById(R.id.connect_button);
        messagesDisplay = findViewById(R.id.messages_display);
        scrollView = findViewById(R.id.scroll_view);

        mainHandler = new Handler(Looper.getMainLooper());

        // Création du client et définition du listener pour les messages reçus
        client = new Client(this);
        client.setMessageListener(new Client.MessageListener() {
            @Override
            public void onMessageReceived(final String message) {
                // Mise à jour de l'UI sur le thread principal
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        appendMessage("Server: " + message);
                    }
                });
            }
        });

        // Gestion du clic sur le bouton de connexion/déconnexion
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!client.isConnected()) {
                    connectToServer();
                } else {
                    disconnectFromServer();
                }
            }
        });

        // Gestion du clic sur le bouton d'envoi
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage();
            }
        });

        updateUI();
    }

    /**
     * Tente de se connecter au serveur dans un thread séparé.
     */
    private void connectToServer() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                final boolean success = client.connectToServer(getIp());

                // Mise à jour de l'UI selon le résultat
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (success) {
                            Toast.makeText(MainActivity.this, "Connected to server",
                                    Toast.LENGTH_SHORT).show();
                            appendMessage("Connected to server");
                        } else {
                            Toast.makeText(MainActivity.this, "Failed to connect",
                                    Toast.LENGTH_SHORT).show();
                            appendMessage("Failed to connect to server");
                        }
                        updateUI();
                    }
                });
            }
        }).start();
    }

    /**
     * Déconnecte le client du serveur dans un thread séparé.
     */
    private void disconnectFromServer() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                client.disconnect();

                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "Disconnected from server",
                                Toast.LENGTH_SHORT).show();
                        appendMessage("Disconnected from server");
                        updateUI();
                    }
                });
            }
        }).start();
    }

    /**
     * Envoie un message au serveur.
     */
    private void sendMessage() {
        final String message = messageInput.getText().toString().trim();

        if (message.isEmpty()) {
            Toast.makeText(this, "Please enter a message", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!client.isConnected()) {
            Toast.makeText(this, "Not connected to server", Toast.LENGTH_SHORT).show();
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                final boolean success = client.sendMessage(message);

                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (success) {
                            appendMessage("You: " + message);
                            messageInput.setText("");
                        } else {
                            Toast.makeText(MainActivity.this, "Failed to send message",
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        }).start();
    }

    /**
     * Récupère l'adresse IP saisie par l'utilisateur.
     * @return L'adresse IP sous forme de chaîne.
     */
    private String getIp() {
        return ipInput.getText().toString().trim();
    }

    /**
     * Ajoute un message à l'affichage et fait défiler vers le bas.
     * @param message Le message à afficher.
     */
    private void appendMessage(String message) {
        messagesDisplay.append(message + "\n\n");
        scrollView.post(new Runnable() {
            @Override
            public void run() {
                scrollView.fullScroll(View.FOCUS_DOWN);
            }
        });
    }

    /**
     * Met à jour l'état de l'UI selon la connexion.
     */
    private void updateUI() {
        boolean connected = client.isConnected();
        sendButton.setEnabled(connected);

        if (connected) {
            connectButton.setText("Disconnect");
            ipInput.setEnabled(false);
        } else {
            connectButton.setText("Connect");
            ipInput.setEnabled(true);
        }
    }

    @Override
    protected void onDestroy() {
        // Déconnexion propre lors de la destruction de l'activité
        if (client != null && client.isConnected()) {
            client.disconnect();
        }
        super.onDestroy();
    }
}