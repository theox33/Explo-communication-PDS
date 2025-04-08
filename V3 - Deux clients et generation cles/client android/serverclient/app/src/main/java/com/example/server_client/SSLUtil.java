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

public class SSLUtil {
    private static final String TAG = "SSLUtil";
    private static Certificate serverCertificate = null;

    /**
     * Extract the primary certificate from a certificate chain
     */
    public static Certificate extractCertificateFromChain(Certificate[] chain) {
        if (chain != null && chain.length > 0) {
            return chain[0]; // Return the first certificate in the chain
        }
        return null;
    }

    /**
     * Update the stored server certificate with a new one
     */
    public static void updateServerCertificate(Certificate newCert) {
        if (newCert != null) {
            Log.d(TAG, "Updating server certificate");
            serverCertificate = newCert;
        }
    }

    /**
     * Save the current server certificate to internal storage
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
     * Update certificate from an input stream
     */
    public static void updateCertificate(InputStream certStream) throws Exception {
        if (certStream != null) {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            serverCertificate = cf.generateCertificate(certStream);
            Log.d(TAG, "Certificate updated from stream");
        }
    }

    /**
     * Get an SSL socket factory configured with our trusted certificate
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