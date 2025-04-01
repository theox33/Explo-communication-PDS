package com.example.server_client;
import android.util.Log;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicBoolean;
public class Connection {
    private static final String TAG = "Connection";
    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private Socket socket;
    private AtomicBoolean connected = new AtomicBoolean(false);
    private InputStream inputStream;
    private OutputStream outputStream;
    public boolean connect(String ip, int port) {
        try {
            Log.d(TAG, "Connecting to " + ip + ":" + port);
            socket = new Socket();
            socket.connect(new InetSocketAddress(ip, port), CONNECT_TIMEOUT);

            // Enable TCP keepalive
            socket.setKeepAlive(true);
            socket.setSoTimeout(2000); // 2 second read timeout

            inputStream = socket.getInputStream();
            outputStream = socket.getOutputStream();
            connected.set(true);

            Log.d(TAG, "Connected successfully");
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Connection failed: " + e.getMessage(), e);
            connected.set(false);
            return false;
        }
    }
    public int read(byte[] buffer, int maxLength) {
        if (!connected.get() || inputStream == null) {
            return -1;
        }

        try {
            return inputStream.read(buffer, 0, maxLength);
        } catch (IOException e) {
            if (e.getMessage() != null &&
                    (e.getMessage().contains("timed out") ||
                            e.getMessage().contains("EAGAIN") ||
                            e.getMessage().contains("EWOULDBLOCK"))) {
                // Timeout, not an error
                return 0;
            }

            Log.e(TAG, "Read failed: " + e.getMessage(), e);
            connected.set(false);
            return -1;
        }
    }
    public boolean write(byte[] data, int length) {
        if (!connected.get() || outputStream == null) {
            return false;
        }

        try {
            outputStream.write(data, 0, length);
            outputStream.flush();
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Write failed: " + e.getMessage(), e);
            connected.set(false);
            return false;
        }
    }
    public void close() {
        connected.set(false);
        try {
            if (socket != null) {
                socket.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "Error closing socket: " + e.getMessage(), e);
        }
    }
    public boolean isConnected() {
        return connected.get() && socket != null && socket.isConnected() && !socket.isClosed();
    }
}