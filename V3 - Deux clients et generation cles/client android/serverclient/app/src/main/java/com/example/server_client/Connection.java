package com.example.server_client;

import android.content.Context;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLHandshakeException;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import java.util.concurrent.atomic.AtomicBoolean;

public class Connection {
    private static final String TAG = "Connection";
    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private SSLSocket socket;
    private AtomicBoolean connected = new AtomicBoolean(false);
    private InputStream inputStream;
    private OutputStream outputStream;
    private Context context;

    public Connection(Context context) {
        this.context = context;
    }

    /**
     * Attempts to connect with a trust manager that accepts any certificate
     * to retrieve a new certificate from the server
     */
    private boolean connectWithNewCertificate(String ip, int port) {
        try {
            Log.d(TAG, "Attempting connection with trust-all certificate manager");

            // Create a trust manager that doesn't validate certificate chains
            TrustManager[] trustAllCerts = new TrustManager[] {
                    new X509TrustManager() {
                        public X509Certificate[] getAcceptedIssuers() {
                            return null;
                        }

                        public void checkClientTrusted(X509Certificate[] certs, String authType) {
                            // Do nothing - trust any client
                        }

                        public void checkServerTrusted(X509Certificate[] certs, String authType) {
                            // Do nothing - trust any server
                        }
                    }
            };

            // Set up SSL context with our trust-all manager
            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, trustAllCerts, new SecureRandom());
            SSLSocketFactory factory = sslContext.getSocketFactory();

            // Create a new socket and connect
            socket = (SSLSocket) factory.createSocket();
            socket.connect(new InetSocketAddress(ip, port), CONNECT_TIMEOUT);
            socket.startHandshake();

            // Get the certificate from the server
            Certificate[] peerCerts = socket.getSession().getPeerCertificates();
            if (peerCerts != null && peerCerts.length > 0) {
                // Update our trust store with this certificate
                Certificate serverCert = peerCerts[0];
                SSLUtil.updateServerCertificate(serverCert);
                SSLUtil.saveServerCertificate(context);

                // Close this insecure connection
                socket.close();
                Log.d(TAG, "Retrieved new certificate, reconnecting securely");

                // Reconnect with the new certificate
                return connect(ip, port);
            } else {
                Log.e(TAG, "Failed to get server certificate");
                return false;
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to connect with new certificate: " + e.getMessage());
            e.printStackTrace();
            connected.set(false);
            return false;
        }
    }

    // Update the connect method for certificate handling
    public boolean connect(String ip, int port) {
        try {
            Log.d(TAG, "Connecting to " + ip + ":" + port);
            SSLSocketFactory factory = SSLUtil.getSocketFactory(context);
            socket = (SSLSocket) factory.createSocket();
            socket.connect(new InetSocketAddress(ip, port), CONNECT_TIMEOUT);

            try {
                // Start handshake to verify certificate
                socket.startHandshake();

                // If handshake succeeds, get the server's certificate chain
                Certificate[] peerCerts = socket.getSession().getPeerCertificates();
                if (peerCerts.length > 0) {
                    // Extract and update the certificate
                    Certificate serverCert = SSLUtil.extractCertificateFromChain(peerCerts);
                    if (serverCert != null) {
                        // Update the certificate in our trust store
                        SSLUtil.updateServerCertificate(serverCert);
                        SSLUtil.saveServerCertificate(context);
                    }
                }

                socket.setKeepAlive(true);
                socket.setSoTimeout(2000); // 2 second read timeout

                inputStream = socket.getInputStream();
                outputStream = socket.getOutputStream();
                connected.set(true);

                // Log connection information
                String cipherSuite = socket.getSession().getCipherSuite();
                Log.d(TAG, "SSL connection established with cipher: " + cipherSuite);

                return true;

            } catch (SSLHandshakeException e) {
                // If handshake fails, attempt to connect with a more permissive approach
                // to get the new certificate
                Log.w(TAG, "SSL handshake failed, attempting to get new certificate: " + e.getMessage());
                try {
                    socket.close();
                } catch (IOException closeErr) {
                    // Ignore close errors
                }

                // Try connecting with new certificate approach
                return connectWithNewCertificate(ip, port);
            }
        } catch (IOException e) {
            Log.e(TAG, "Connection failed: " + e.getMessage());
            e.printStackTrace();
            connected.set(false);
            return false;
        } catch (Exception e) {
            Log.e(TAG, "Certificate handling error: " + e.getMessage());
            e.printStackTrace();
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
