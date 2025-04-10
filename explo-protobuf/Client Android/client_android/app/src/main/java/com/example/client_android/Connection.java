package com.example.client_android;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.example.client_android.Message;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

public class Connection extends AppCompatActivity {

    private Button connectButton, sendButton;
    private EditText messageInput;
    private TextView messagesDisplay;
    private ScrollView scrollView;

    private Socket socket;
    private OutputStream out;
    private Thread receiveThread;
    private boolean isConnected = false;

    private final String SERVER_IP = "10.247.95.45";  // change if needed
    private final int SERVER_PORT = 12345;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main); // Assure-toi que le XML s'appelle bien comme ça

        connectButton = findViewById(R.id.connect_button);
        sendButton = findViewById(R.id.send_button);
        messageInput = findViewById(R.id.message_input);
        messagesDisplay = findViewById(R.id.messages_display);
        scrollView = findViewById(R.id.scroll_view);

        sendButton.setEnabled(false);

        connectButton.setOnClickListener(v -> {
            if (!isConnected) {
                connectToServer();
            } else {
                disconnectFromServer();
            }
        });

        sendButton.setOnClickListener(v -> {
            String messageText = messageInput.getText().toString();
            if (!messageText.isEmpty()) {
                sendMessage(messageText);
                messageInput.setText("");
                appendMessage("You: " + messageText);
            }
        });
    }

    private void connectToServer() {
        new Thread(() -> {
            try {
                socket = new Socket(SERVER_IP, SERVER_PORT);
                out = socket.getOutputStream();
                isConnected = true;

                runOnUiThread(() -> {
                    connectButton.setText("Disconnect");
                    sendButton.setEnabled(true);
                    appendMessage("[Connected to server]");
                });

                receiveThread = new Thread(() -> {
                    try {
                        InputStream input = socket.getInputStream();
                        byte[] buffer = new byte[1024];
                        while (isConnected) {
                            int bytesRead = input.read(buffer);
                            if (bytesRead == -1) break;

                            byte[] actualData = new byte[bytesRead];
                            System.arraycopy(buffer, 0, actualData, 0, bytesRead);

                            Message.AMessage msg = Message.AMessage.parseFrom(actualData);
                            String decrypted = xorTransform(msg.getContent());

                            runOnUiThread(() -> appendMessage("Server: " + decrypted));
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

    private void disconnectFromServer() {
        try {
            isConnected = false;
            if (socket != null) socket.close();
            if (receiveThread != null && receiveThread.isAlive()) receiveThread.interrupt();

            runOnUiThread(() -> {
                connectButton.setText("Connect");
                sendButton.setEnabled(false);
                appendMessage("[Disconnected from server]");
            });

        } catch (Exception e) {
            runOnUiThread(() -> appendMessage("[Disconnection error: " + e.getMessage() + "]"));
            e.printStackTrace();
        }
    }

    private void sendMessage(String messageText) {
        new Thread(() -> {
            try {
                Message.AMessage protoMessage = Message.AMessage.newBuilder()
                        .setContent(xorTransform(messageText))
                        .build();

                byte[] data = protoMessage.toByteArray();
                out.write(data);
                out.flush();
            } catch (Exception e) {
                runOnUiThread(() -> appendMessage("[Send error: " + e.getMessage() + "]"));
                e.printStackTrace();
            }
        }).start();
    }

    private void appendMessage(String message) {
        messagesDisplay.append(message + "\n");
        scrollView.post(() -> scrollView.fullScroll(View.FOCUS_DOWN));
    }

    private String xorTransform(String input) {
        char[] chars = input.toCharArray();
        for (int i = 0; i < chars.length; i++) {
            chars[i] ^= 0x5A;
        }
        return new String(chars);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnectFromServer();
    }
}
