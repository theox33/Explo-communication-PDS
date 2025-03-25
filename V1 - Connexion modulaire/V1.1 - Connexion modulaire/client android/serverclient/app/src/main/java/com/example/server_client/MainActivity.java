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
public class MainActivity extends AppCompatActivity {
    private Client client;
    private EditText messageInput;
    private Button sendButton;
    private Button connectButton;
    private TextView messagesDisplay;
    private ScrollView scrollView;
    private Handler mainHandler;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize UI components
        messageInput = findViewById(R.id.message_input);
        sendButton = findViewById(R.id.send_button);
        connectButton = findViewById(R.id.connect_button);
        messagesDisplay = findViewById(R.id.messages_display);
        scrollView = findViewById(R.id.scroll_view);

        mainHandler = new Handler(Looper.getMainLooper());

        // Create client instance
        client = new Client();
        client.setMessageListener(new Client.MessageListener() {
            @Override
            public void onMessageReceived(final String message) {
                // Update UI on main thread
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        appendMessage("Server: " + message);
                    }
                });
            }
        });

        // Set up connect button
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

        // Set up send button
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage();
            }
        });

        updateUI();
    }

    private void connectToServer() {
        // Connection should be done in a background thread
        new Thread(new Runnable() {
            @Override
            public void run() {
                final boolean success = client.connectToServer();

                // Update UI on main thread
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

    private void appendMessage(String message) {
        messagesDisplay.append(message + "\n\n");
        scrollView.post(new Runnable() {
            @Override
            public void run() {
                scrollView.fullScroll(View.FOCUS_DOWN);
            }
        });
    }

    private void updateUI() {
        boolean connected = client.isConnected();
        sendButton.setEnabled(connected);

        if (connected) {
            connectButton.setText("Disconnect");
        } else {
            connectButton.setText("Connect");
        }
    }

    @Override
    protected void onDestroy() {
        if (client != null && client.isConnected()) {
            client.disconnect();
        }
        super.onDestroy();
    }
}