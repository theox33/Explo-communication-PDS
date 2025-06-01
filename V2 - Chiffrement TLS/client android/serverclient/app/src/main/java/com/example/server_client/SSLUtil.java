package com.example.server_client;

import android.content.Context;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;

/**
 * @class SSLUtil
 * @brief Utilitaire pour créer une SSLSocketFactory personnalisée avec un certificat auto-signé.
 */
public class SSLUtil {

    /**
     * @brief Retourne une SSLSocketFactory configurée avec un certificat de confiance personnalisé.
     * @param context Le contexte Android pour accéder aux assets.
     * @return Une instance de SSLSocketFactory prête à l'emploi.
     */
    public static SSLSocketFactory getSocketFactory(Context context) {
        try {
            // Charge le certificat auto-signé depuis les assets
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            InputStream caInput = context.getAssets().open("server.crt");
            Certificate ca;
            try {
                ca = cf.generateCertificate(caInput);
            } finally {
                caInput.close();
            }

            // Crée un KeyStore contenant le certificat de confiance
            String keyStoreType = KeyStore.getDefaultType();
            KeyStore keyStore = KeyStore.getInstance(keyStoreType);
            keyStore.load(null, null);
            keyStore.setCertificateEntry("server", ca);

            // Crée un TrustManager qui fait confiance au certificat du KeyStore
            String tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm();
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(tmfAlgorithm);
            tmf.init(keyStore);

            // Crée un SSLContext utilisant ce TrustManager
            SSLContext contextSSL = SSLContext.getInstance("TLS");
            contextSSL.init(null, tmf.getTrustManagers(), null);
            return contextSSL.getSocketFactory();
        } catch (Exception e) {
            e.printStackTrace();
            return (SSLSocketFactory) SSLSocketFactory.getDefault();
        }
    }
}