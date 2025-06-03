/**
 * @file TLSSocketFactory.java
 * @brief Custom TLS socket factory with certificate pinning
 * @version 4.0
 * @author Alexis DEVERCHERE
 *
 * @section description Description
 * Creates secure SSL sockets with custom trust management and
 * certificate pinning for enhanced security.
 *
 * @section security Security Features
 * - TLS 1.2/1.3 enforcement
 * - Certificate pinning
 * - Secure cipher suites
 */

package com.example.securetlschat;

import android.content.Context;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

public class TLSSocketFactory {
    private static final String TAG = "TLSSocketFactory";
    private SSLSocketFactory sslSocketFactory;
    private final String serverHost;
    private final int serverPort;

    /**
     * @brief Constructor
     * @param context Android context
     * @param host Server hostname
     * @param port Server port
     */
    public TLSSocketFactory(Context context, String host, int port) {
        this.serverHost = host;
        this.serverPort = port;
        initializeSSLContext(context);
    }

    /**
     * @brief Initialize SSL context with pinned certificate
     * @param context Android context
     */
    private void initializeSSLContext(Context context) {
        try {
            // Load pinned certificate from resources
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            InputStream certInput = new BufferedInputStream(
                    context.getResources().openRawResource(R.raw.server_cert)
            );

            Certificate ca;
            try {
                ca = cf.generateCertificate(certInput);
                Log.d(TAG, "Certificate loaded: " + ((X509Certificate) ca).getSubjectDN());
            } finally {
                certInput.close();
            }

            // Create KeyStore with our trusted certificate
            String keyStoreType = KeyStore.getDefaultType();
            KeyStore keyStore = KeyStore.getInstance(keyStoreType);
            keyStore.load(null, null);
            keyStore.setCertificateEntry("server", ca);

            // Create TrustManager that trusts our certificate
            String tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm();
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(tmfAlgorithm);
            tmf.init(keyStore);

            // Create SSLContext that uses our TrustManager
            SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
            sslContext.init(null, tmf.getTrustManagers(), null);

            this.sslSocketFactory = sslContext.getSocketFactory();

        } catch (CertificateException | KeyStoreException | NoSuchAlgorithmException |
                 IOException | KeyManagementException e) {
            Log.e(TAG, "Error initializing SSL context", e);
            throw new RuntimeException("Failed to initialize SSL context", e);
        }
    }

    /**
     * @brief Create secure socket with TLS configuration
     * @return SSLSocket Configured SSL socket
     * @throws IOException On socket creation error
     */
    public SSLSocket createSocket() throws IOException {
        SSLSocket socket = (SSLSocket) sslSocketFactory.createSocket(serverHost, serverPort);

        // Configure TLS parameters
        socket.setEnabledProtocols(new String[]{"TLSv1.2", "TLSv1.3"});

        // Recommended cipher suites
        String[] cipherSuites = {
                "TLS_AES_128_GCM_SHA256",
                "TLS_AES_256_GCM_SHA384",
                "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256",
                "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384"
        };
        socket.setEnabledCipherSuites(cipherSuites);

        return socket;
    }

    /**
     * @brief Create debug socket factory (INSECURE - for testing only)
     * @return SSLSocketFactory Factory that accepts all certificates
     * @warning Never use in production!
     */
    public static SSLSocketFactory createDebugSocketFactory() {
        try {
            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, new TrustManager[]{new X509TrustManager() {
                @Override
                public void checkClientTrusted(X509Certificate[] chain, String authType) {}

                @Override
                public void checkServerTrusted(X509Certificate[] chain, String authType) {}

                @Override
                public X509Certificate[] getAcceptedIssuers() {
                    return new X509Certificate[0];
                }
            }}, null);

            return sslContext.getSocketFactory();
        } catch (Exception e) {
            throw new RuntimeException("Failed to create debug SSL factory", e);
        }
    }
}