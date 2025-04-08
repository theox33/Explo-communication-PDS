package com.example.client_android;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Connection extends AppCompatActivity {

    private EditText editTextMessage;
    private Button sendButton;
    private TextView textViewStatus;
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    // Update the IP address as needed.
    // If testing on an emulator with the server running on your local PC, use "10.0.2.2".
    private final String SERVER_IP = "192.168.242.45";
    private final int SERVER_PORT = 12345;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Link the UI components defined in activity_main.xml
        editTextMessage = findViewById(R.id.editTextMessage);
        sendButton = findViewById(R.id.sendButton);
        textViewStatus = findViewById(R.id.textViewStatus);

        // Connect to the server on a background thread
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    socket = new Socket(SERVER_IP, SERVER_PORT);
                    out = new PrintWriter(socket.getOutputStream(), true);
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            textViewStatus.setText("Connected to server " + SERVER_IP + ":" + SERVER_PORT);
                        }
                    });

                    // Optionally, read messages from the server if needed.
                    // This sample only sends messages.

                } catch (final Exception e) {
                    e.printStackTrace();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            textViewStatus.setText("Connection error: " + e.getMessage());
                        }
                    });
                }
            }
        }).start();

        // Set up the send button to transmit messages to the server
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                final String message = editTextMessage.getText().toString();
                if (message.isEmpty()) return;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (out != null) {
                            out.println(message);
                        }
                    }
                }).start();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Clean up the socket when the activity is destroyed
        try {
            if (socket != null) {
                socket.close();
            }
        } catch (Exception e) {
            // Exception handling as needed
        }
    }
}
