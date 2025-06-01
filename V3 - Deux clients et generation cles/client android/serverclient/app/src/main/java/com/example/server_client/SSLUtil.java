/**
 * @file SSLUtil.java
 * @brief SSL/TLS certificate management utilities for Android
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

package com.example.server_client;
import android.content.Context;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.CertificateException;

import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;

/**
 * @brief Utility class for SSL/TLS certificate management and socket factory creation
 * 
 * This class provides functionality for managing server certificates dynamically,
 * including extraction from certificate chains, updating stored certificates,
 * and creating SSL socket factories with custom trust stores.
 */
public class SSLUtil {
    /** @brief Tag for Android logging */
    private static final String TAG = "SSLUtil";
    
    /** @brief Currently stored server certificate */
    private static Certificate serverCertificate = null;

    /**
     * @brief Extract the primary certificate from a certificate chain
     * 
     * Retrieves the first (leaf) certificate from a certificate chain,
     * which typically represents the server's identity certificate.
     * 
     * @param chain Array of certificates forming the chain
     * @return Certificate The primary certificate, or null if chain is empty
     * 
     * @note The first certificate in the chain is usually the server certificate
     * @note Returns null for null or empty certificate chains
     * @note Subsequent certificates in the chain are intermediate/root CAs
     */
    public static Certificate extractCertificateFromChain(Certificate[] chain) {
        if (chain != null && chain.length > 0) {
            return chain[0]; // Return the first certificate in the chain
        }
        return null;
    }

    /**
     * @brief Update the stored server certificate with a new one
     * 
     * Replaces the currently stored server certificate with a new certificate,
     * typically obtained from a recent connection to the server.
     * 
     * @param newCert The new certificate to store
     * 
     * @note Only updates if the new certificate is not null
     * @note The certificate is stored in static memory until the app restarts
     * @note Should be followed by saveServerCertificate() to persist to storage
     */
    public static void updateServerCertificate(Certificate newCert) {
        if (newCert != null) {
            Log.d(TAG, "Updating server certificate");
            serverCertificate = newCert;
        }
    }

    /**
     * @brief Save the current server certificate to internal storage
     * 
     * Persists the currently stored server certificate to the application's
     * internal storage for retrieval across app restarts.
     * 
     * @param context Android application context for file operations
     * 
     * @note Saves to internal storage as "server_current.crt"
     * @note Requires a certificate to be stored via updateServerCertificate()
     * @note Logs errors if certificate encoding or file operations fail
     */
    public static void saveServerCertificate(Context context) {
        if (serverCertificate == null) {
            Log.e(TAG, "No server certificate to save");
            return;
        }

        try {
            File certFile = new File(context.getFilesDir(), "server_current.crt");
            FileOutputStream outputStream = new FileOutputStream(certFile);
            outputStream.write(serverCertificate.getEncoded());
            outputStream.close();
            Log.d(TAG, "Server certificate saved to: " + certFile.getAbsolutePath());
        } catch (IOException | java.security.cert.CertificateEncodingException e) {
            Log.e(TAG, "Error saving server certificate: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * @brief Update certificate from an input stream
     * 
     * Loads a certificate from an input stream and updates the stored
     * server certificate. Useful for loading certificates from files or assets.
     * 
     * @param certStream Input stream containing X.509 certificate data
     * @throws Exception If certificate parsing or stream reading fails
     * 
     * @note Expects X.509 format certificate data
     * @note Replaces any previously stored certificate
     * @note Stream is not closed by this method
     */
    public static void updateCertificate(InputStream certStream) throws Exception {
        if (certStream != null) {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            serverCertificate = cf.generateCertificate(certStream);
            Log.d(TAG, "Certificate updated from stream");
        }
    }

    /**
     * @brief Get an SSL socket factory configured with trusted certificates
     * 
     * Creates an SSL socket factory that trusts the stored server certificate.
     * If no certificate is stored, loads the initial certificate from assets.
     * Falls back to default socket factory on errors.
     * 
     * @param context Android application context for accessing assets
     * @return SSLSocketFactory Configured factory for creating SSL sockets
     * 
     * @note Uses stored certificate if available, otherwise loads from assets/server.crt
     * @note Creates a custom KeyStore containing only the trusted server certificate
     * @note Falls back to default socket factory if configuration fails
     * @note The returned factory validates server certificates against the stored cert
     */
    public static SSLSocketFactory getSocketFactory(Context context) {
        try {
            // If we haven't received a certificate from the server yet,
            // fall back to the bundled one
            if (serverCertificate == null) {
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                InputStream caInput = context.getAssets().open("server.crt");
                try {
                    serverCertificate = cf.generateCertificate(caInput);
                    Log.d(TAG, "Using initial certificate from assets");
                } finally {
                    caInput.close();
                }
            } else {
                Log.d(TAG, "Using existing certificate");
            }

            // Create a KeyStore containing our trusted CA
            String keyStoreType = KeyStore.getDefaultType();
            KeyStore keyStore = KeyStore.getInstance(keyStoreType);
            keyStore.load(null, null);
            keyStore.setCertificateEntry("server", serverCertificate);

            // Create a TrustManager that trusts the CAs in our KeyStore
            String tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm();
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(tmfAlgorithm);
            tmf.init(keyStore);

            // Create an SSLContext that uses our TrustManager
            SSLContext contextSSL = SSLContext.getInstance("TLS");
            contextSSL.init(null, tmf.getTrustManagers(), null);
            return contextSSL.getSocketFactory();
        } catch (Exception e) {
            Log.e(TAG, "Error creating SSL socket factory: " + e.getMessage());
            e.printStackTrace();
            return (SSLSocketFactory) SSLSocketFactory.getDefault();
        }
    }
}