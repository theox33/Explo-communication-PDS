/**
 * @file MainActivity.java
 * @brief Main activity for Secure TLS Chat Android application
 * @version 4.0
 * @author Alexis DEVERCHERE
 *
 * @section description Description
 * This activity provides the main user interface for the secure chat client,
 * handling connection management, message display, and user input.
 *
 * @section features Features
 * - Server connection management
 * - Real-time message display with timestamps
 * - User-friendly interface with auto-scrolling
 * - Connection status visualization
 */

package com.example.securetlschat;

import android.graphics.Color;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {

    private EditText etServerIp, etServerPort, etMessage;
    private TextView tvConnectionStatus, tvMessages;
    private Button btnConnect, btnSend;
    private ScrollView scrollView;

    private SecureChatClient chatClient;
    private final SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm:ss", Locale.getDefault());

    /**
     * @brief Initialize activity and UI components
     * @param savedInstanceState Saved instance state
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initializeViews();
        setupListeners();
    }

    /**
     * @brief Initialize all view components
     */
    private void initializeViews() {
        etServerIp = findViewById(R.id.etServerIp);
        etServerPort = findViewById(R.id.etServerPort);
        etMessage = findViewById(R.id.etMessage);
        tvConnectionStatus = findViewById(R.id.tvConnectionStatus);
        tvMessages = findViewById(R.id.tvMessages);
        btnConnect = findViewById(R.id.btnConnect);
        btnSend = findViewById(R.id.btnSend);
        scrollView = findViewById(R.id.scrollView);

        tvMessages.setMovementMethod(new ScrollingMovementMethod());
    }

    /**
     * @brief Set up event listeners for UI components
     */
    private void setupListeners() {
        btnConnect.setOnClickListener(v -> {
            if (chatClient != null && chatClient.isConnected()) {
                disconnect();
            } else {
                connect();
            }
        });

        btnSend.setOnClickListener(v -> sendMessage());

        etMessage.setOnEditorActionListener((v, actionId, event) -> {
            sendMessage();
            return true;
        });
    }

    /**
     * @brief Establish connection to chat server
     */
    private void connect() {
        String host = etServerIp.getText().toString().trim();
        String portStr = etServerPort.getText().toString().trim();

        if (host.isEmpty() || portStr.isEmpty()) {
            Toast.makeText(this, "Please enter server IP and port", Toast.LENGTH_SHORT).show();
            return;
        }

        try {
            int port = Integer.parseInt(portStr);

            chatClient = new SecureChatClient(this, host, port);

            chatClient.setConnectionListener(new SecureChatClient.ConnectionListener() {
                @Override
                public void onConnected() {
                    runOnUiThread(() -> {
                        updateConnectionStatus(true);
                        appendMessage("System", "Connected to server");
                    });
                }

                @Override
                public void onDisconnected() {
                    runOnUiThread(() -> {
                        updateConnectionStatus(false);
                        appendMessage("System", "Disconnected from server");
                    });
                }

                @Override
                public void onConnectionError(String error) {
                    runOnUiThread(() -> {
                        updateConnectionStatus(false);
                        appendMessage("Error", error);
                        Toast.makeText(MainActivity.this, "Connection error: " + error,
                                Toast.LENGTH_LONG).show();
                    });
                }
            });

            chatClient.setMessageListener(new SecureChatClient.MessageListener() {
                @Override
                public void onMessageReceived(String message) {
                    runOnUiThread(() -> appendMessage("Server", message));
                }

                @Override
                public void onError(String error) {
                    runOnUiThread(() -> appendMessage("Error", error));
                }
            });

            chatClient.connect();
            updateConnectionStatus(false);
            tvConnectionStatus.setText("Connecting...");

        } catch (NumberFormatException e) {
            Toast.makeText(this, "Invalid port number", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * @brief Disconnect from chat server
     */
    private void disconnect() {
        if (chatClient != null) {
            chatClient.disconnect();
            chatClient = null;
        }
    }

    /**
     * @brief Send message to chat server
     */
    private void sendMessage() {
        String message = etMessage.getText().toString().trim();
        if (message.isEmpty()) return;

        if (chatClient != null && chatClient.isConnected()) {
            chatClient.sendMessage(message);
            appendMessage("You", message);
            etMessage.setText("");
        } else {
            Toast.makeText(this, "Not connected to server", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * @brief Append message to chat display
     * @param sender Message sender
     * @param message Message content
     */
    private void appendMessage(String sender, String message) {
        String timestamp = dateFormat.format(new Date());
        String formattedMessage = String.format("[%s] %s: %s\n", timestamp, sender, message);

        tvMessages.append(formattedMessage);

        // Auto-scroll to bottom
        scrollView.post(() -> scrollView.fullScroll(View.FOCUS_DOWN));
    }

    /**
     * @brief Update connection status UI
     * @param connected Current connection state
     */
    private void updateConnectionStatus(boolean connected) {
        if (connected) {
            tvConnectionStatus.setText("Connected");
            tvConnectionStatus.setTextColor(Color.GREEN);
            btnConnect.setText("Disconnect");
            etMessage.setEnabled(true);
            btnSend.setEnabled(true);
            etServerIp.setEnabled(false);
            etServerPort.setEnabled(false);
        } else {
            tvConnectionStatus.setText("Disconnected");
            tvConnectionStatus.setTextColor(Color.RED);
            btnConnect.setText("Connect");
            etMessage.setEnabled(false);
            btnSend.setEnabled(false);
            etServerIp.setEnabled(true);
            etServerPort.setEnabled(true);
        }
    }

    /**
     * @brief Clean up resources on activity destruction
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (chatClient != null) {
            chatClient.destroy();
        }
    }
}