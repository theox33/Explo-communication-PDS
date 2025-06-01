/**
 * @file MainActivity.java
 * @brief Main Android activity for the messaging client application
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

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
 * @brief Main activity providing the user interface for the messaging client
 * 
 * This activity manages the user interface for connecting to the server,
 * sending messages, and displaying received messages. It handles all UI
 * interactions and coordinates with the Client class for network operations.
 */
public class MainActivity extends AppCompatActivity {
    /** @brief Client instance for server communication */
    private Client client;
    
    /** @brief Input field for typing messages */
    private EditText messageInput;
    
    /** @brief Button for sending messages */
    private Button sendButton;
    
    /** @brief Button for connecting/disconnecting */
    private Button connectButton;
    
    /** @brief Text view for displaying messages */
    private TextView messagesDisplay;
    
    /** @brief Scroll view containing the messages display */
    private ScrollView scrollView;
    
    /** @brief Handler for updating UI from background threads */
    private Handler mainHandler;

    /**
     * @brief Called when the activity is first created
     * 
     * Initializes the user interface components, creates the client instance,
     * sets up event listeners, and configures message handling.
     * 
     * @param savedInstanceState Bundle containing saved state (unused)
     * 
     * @note All UI updates from background threads are posted to mainHandler
     * @note The client is initialized with this activity's context
     * @note Message listener automatically updates the UI when messages arrive
     */
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
        client = new Client(this);
        client.setMessageListener(new Client.MessageListener() {
            @Override
            public void onMessageReceived(final String message, final String senderId) {
                // Update UI on main thread
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        appendMessage("Client " + senderId + ": " + message);
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

    /**
     * @brief Initiate connection to the server in a background thread
     * 
     * Performs the connection operation in a background thread to avoid
     * blocking the UI. Updates the interface based on connection success
     * or failure.
     * 
     * @note Connection is performed asynchronously to prevent ANR
     * @note UI updates are posted back to the main thread
     * @note Shows toast messages for connection status feedback
     */
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

    /**
     * @brief Disconnect from the server in a background thread
     * 
     * Performs the disconnection operation in a background thread and
     * updates the UI accordingly.
     * 
     * @note Disconnection is performed asynchronously for consistency
     * @note UI updates are posted back to the main thread
     * @note Shows toast message and updates message display
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
     * @brief Send a message to the server
     * 
     * Validates the input message, sends it to the server in a background
     * thread, and updates the UI based on the result.
     * 
     * @note Validates that message is not empty before sending
     * @note Checks connection status before attempting to send
     * @note Message sending is performed asynchronously
     * @note Clears input field on successful send
     * @note Displays sent message in the message history
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
     * @brief Append a message to the message display area
     * 
     * Adds a new message to the scrollable text view and automatically
     * scrolls to the bottom to show the latest message.
     * 
     * @param message The message text to display
     * 
     * @note Automatically scrolls to bottom after adding message
     * @note Messages are separated by double newlines for readability
     * @note Scroll operation is posted to ensure proper timing
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
     * @brief Update the user interface based on connection status
     * 
     * Enables/disables buttons and updates button text based on whether
     * the client is currently connected to the server.
     * 
     * @note Send button is only enabled when connected
     * @note Connect button text changes based on connection status
     * @note Called whenever connection status might have changed
     */
    private void updateUI() {
        boolean connected = client.isConnected();
        sendButton.setEnabled(connected);

        if (connected) {
            connectButton.setText("Disconnect");
        } else {
            connectButton.setText("Connect");
        }
    }

    /**
     * @brief Called when the activity is being destroyed
     * 
     * Ensures proper cleanup by disconnecting from the server if still
     * connected before the activity is destroyed.
     * 
     * @note Prevents resource leaks by ensuring proper disconnection
     * @note Called automatically by the Android framework
     * @note Always calls super.onDestroy() for proper cleanup chain
     */
    @Override
    protected void onDestroy() {
        if (client != null && client.isConnected()) {
            client.disconnect();
        }
        super.onDestroy();
    }
}