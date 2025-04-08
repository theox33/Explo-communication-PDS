package com.example.ours;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    private static final String SERVER_IP = "172.23.2.137"; // Remplace avec l'IP de ton serveur
    private static final int SERVER_PORT = 12345;
    private Socket socket;
    private BufferedReader input;
    private OutputStream output;
    private TextView conversationTextView;
    private EditText messageEditText;
    private Button sendButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        conversationTextView = findViewById(R.id.conversationTextView);
        messageEditText = findViewById(R.id.messageEditText);
        sendButton = findViewById(R.id.sendButton);

        // Écoute le bouton "Envoyer"
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String message = messageEditText.getText().toString();
                if (!message.isEmpty()) {
                    sendMessageToServer(message);
                }
            }
        });

        // Connexion au serveur dès le lancement de l'application
        connectToServer();
    }

    // Méthode pour établir une connexion au serveur
    private void connectToServer() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    socket = new Socket(SERVER_IP, SERVER_PORT);  // Connexion au serveur
                    input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    output = socket.getOutputStream();

                    Log.d("TCPClient", "Connexion réussie au serveur");

                    // Lire les messages entrants du serveur
                    String serverMessage;
                    while ((serverMessage = input.readLine()) != null) {
                        final String messageToDisplay = serverMessage;  // variable finale
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                conversationTextView.append("Serveur: " + messageToDisplay + "\n");
                            }
                        });
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, "Erreur de connexion: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                            Log.e("TCPClient", "Erreur lors de la connexion : " + e.getMessage());
                        }
                    });
                }
            }
        }).start();
    }

    // Méthode pour envoyer un message au serveur
    private void sendMessageToServer(String message) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (socket != null && !socket.isClosed()) {
                        output.write((message + "\n").getBytes()); // Envoi du message
                        output.flush();

                        // Affiche le message envoyé dans la conversation
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                conversationTextView.append("Vous: " + message + "\n");
                                messageEditText.setText("");  // Effacer le champ de saisie
                            }
                        });
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, "Erreur d'envoi du message", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            if (socket != null) {
                socket.close(); // Fermer la connexion lors de la destruction de l'activité
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
