package com.example.server_client;
import android.util.Log;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;
public class Communication {
    private static final String TAG = "Communication";
    private static final int BUFFER_SIZE = 1024;

    public interface MessageHandler {
        void onMessageReceived(String cmd, String param);
    }

    private Connection connection;
    private Protocol protocol;
    private MessageHandler messageHandler;
    private Thread communicationThread;
    private AtomicBoolean running = new AtomicBoolean(false);

    public Communication(Connection connection, Protocol protocol) {
        this.connection = connection;
        this.protocol = protocol;
    }

    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler;
    }

    public void comX(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_X, param);
        connection.write(message, message.length);
    }

    public void comY(String param) {
        byte[] message = protocol.encodeMessage(Protocol.CMD_Y, param);
        connection.write(message, message.length);connection.write(message, message.length);
    }

    public void run() {
        if (running.get()) {
            return; // Already running
        }

        running.set(true);

        communicationThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Communication thread started");
                byte[] buffer = new byte[BUFFER_SIZE];
                int consecutiveEmptyReads = 0;
                final int maxConsecutiveEmptyReads = 5;

                while (running.get() && connection.isConnected()) {
                    Arrays.fill(buffer, (byte)0); // Clear buffer

                    int bytesRead = connection.read(buffer, BUFFER_SIZE - 1);

                    if (bytesRead > 0) {
                        consecutiveEmptyReads = 0;

                        // Ensure null termination and convert to string
                        String rawMessage = new String(buffer, 0, bytesRead);
                        Log.d(TAG, "Raw data received: '" + rawMessage + "'");

                        // Process the message
                        String[] parts = protocol.decodeMessage(rawMessage);

                        // Call the message handler if set
                        if (messageHandler != null) {
                            messageHandler.onMessageReceived(parts[0], parts[1]);
                        }
                    } else if (bytesRead == 0) {
                        // No data available
                        consecutiveEmptyReads++;

                        // If connection lost
                        if (!connection.isConnected()) {
                            Log.d(TAG, "Connection closed by peer (normal)");
                            break;
                        }

                        // Connection check after multiple empty reads
                        if (consecutiveEmptyReads > maxConsecutiveEmptyReads) {
                            Log.d(TAG, "Multiple empty reads, checking connection");
                            if (!connection.isConnected()) {
                                Log.d(TAG, "Connection appears broken");
                                break;
                            }
                            consecutiveEmptyReads = 0;
                        }
                    } else {
                        // Error occurred
                        Log.e(TAG, "Read error, terminating communication thread");
                        break;
                    }

                    // Small delay to prevent CPU hogging
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

    public void stop() {
        running.set(false);
        if (communicationThread != null) {
            communicationThread.interrupt();
            try {
                communicationThread.join(1000); // Wait up to 1 second for thread to finish
            } catch (InterruptedException e) {
                Log.w(TAG, "Interrupted while waiting for thread to join");
            }
            communicationThread = null;
        }
    }

    public boolean isRunning() {
        return running.get();
    }
}