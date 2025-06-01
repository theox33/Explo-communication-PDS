/**
 * @file Connection.java
 * @brief SSL/TLS network connection management for Android
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

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

/**
 * @brief SSL/TLS connection manager with dynamic certificate handling
 * 
 * This class manages secure network connections using SSL/TLS with support
 * for dynamic certificate retrieval and trust store updates. It handles
 * certificate validation failures by attempting to retrieve new certificates
 * from the server.
 */
public class Connection {
    /** @brief Tag for Android logging */
    private static final String TAG = "Connection";
    
    /** @brief Connection timeout in milliseconds */
    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    
    /** @brief SSL socket for secure communication */
    private SSLSocket socket;
    
    /** @brief Thread-safe connection status flag */
    private AtomicBoolean connected = new AtomicBoolean(false);
    
    /** @brief Input stream for reading data */
    private InputStream inputStream;
    
    /** @brief Output stream for writing data */
    private OutputStream outputStream;
    
    /** @brief Android application context */
    private Context context;

    /**
     * @brief Constructor for Connection class
     * 
     * Initializes the connection manager with the provided Android context
     * which is required for certificate management and file operations.
     * 
     * @param context Android application context
     * 
     * @note The context is used for accessing assets and internal storage
     * @note No network connection is established until connect() is called
     */
    public Connection(Context context) {
        this.context = context;
    }

    /**
     * @brief Attempt connection with permissive certificate validation
     * 
     * This method creates a connection that accepts any certificate to retrieve
     * a new certificate from the server. It then updates the trust store and
     * reconnects securely with the new certificate.
     * 
     * @param ip Server IP address
     * @param port Server port number
     * @return boolean True if connection was successful, false otherwise
     * 
     * @note This method bypasses certificate validation temporarily
     * @note Used as fallback when normal SSL handshake fails
     * @note Automatically reconnects securely after certificate retrieval
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

    /**
     * @brief Establish a secure connection to the server
     * 
     * Creates an SSL/TLS connection using the configured trust store. If the
     * SSL handshake fails due to certificate issues, it attempts to retrieve
     * a new certificate from the server and reconnect.
     * 
     * @param ip Server IP address
     * @param port Server port number
     * @return boolean True if connection was successful, false otherwise
     * 
     * @note Implements automatic certificate retrieval on handshake failure
     * @note Configures socket with keepalive and read timeout
     * @note Logs the cipher suite used for the connection
     */
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

    /**
     * @brief Read data from the connection
     * 
     * Attempts to read data from the SSL socket input stream into the
     * provided buffer. Handles timeout and error conditions gracefully.
     * 
     * @param buffer Byte array to store received data
     * @param maxLength Maximum number of bytes to read
     * @return int Number of bytes read, 0 for timeout, -1 for error/disconnection
     * 
     * @note Timeout conditions return 0 to allow retry
     * @note Network errors result in connection being marked as disconnected
     * @note Thread-safe operation using atomic boolean for connection state
     */
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

    /**
     * @brief Write data to the connection
     * 
     * Sends data through the SSL socket output stream. The data is
     * immediately flushed to ensure transmission.
     * 
     * @param data Byte array containing data to send
     * @param length Number of bytes to write from the array
     * @return boolean True if write was successful, false otherwise
     * 
     * @note Output is automatically flushed after writing
     * @note Write failures result in connection being marked as disconnected
     * @note Thread-safe operation using atomic boolean for connection state
     */
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

    /**
     * @brief Close the connection and clean up resources
     * 
     * Gracefully closes the SSL socket and marks the connection as
     * disconnected. Safe to call multiple times.
     * 
     * @note Thread-safe operation
     * @note Does not throw exceptions on close errors
     * @note Connection state is immediately updated
     */
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

    /**
     * @brief Check if the connection is currently active
     * 
     * Verifies both the internal connection state and the socket status
     * to determine if the connection is usable.
     * 
     * @return boolean True if connected and socket is open, false otherwise
     * 
     * @note Thread-safe using atomic boolean
     * @note Checks both internal state and actual socket status
     * @note Connection state can change asynchronously due to network events
     */
    public boolean isConnected() {
        return connected.get() && socket != null && socket.isConnected() && !socket.isClosed();
    }
}