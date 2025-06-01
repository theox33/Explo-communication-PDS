package com.example.server_client;

import android.content.Context;
import android.util.Log;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Classe qui gère la connexion SSL, la lecture et l'écriture de données avec le serveur.
 */
public class Connection {
    private static final String TAG = "Connection";
    private static final int CONNECT_TIMEOUT = 5000; // Timeout de connexion en ms

    private SSLSocket socket; // Socket SSL pour la connexion sécurisée
    private AtomicBoolean connected = new AtomicBoolean(false); // Indique si la connexion est active
    private InputStream inputStream; // Flux d'entrée pour lire les données
    private OutputStream outputStream; // Flux de sortie pour écrire les données
    private Context context; // Contexte Android pour accéder aux ressources

    /**
     * Constructeur de la classe Connection.
     * @param context Le contexte Android.
     */
    public Connection(Context context) {
        this.context = context;
    }

    /**
     * Tente d'établir une connexion SSL avec le serveur.
     * @param ip L'adresse IP du serveur.
     * @param port Le port du serveur.
     * @return true si la connexion a réussi, false sinon.
     */
    public boolean connect(String ip, int port) {
        try {
            Log.d(TAG, "Connecting to " + ip + ":" + port);
            SSLSocketFactory factory = SSLUtil.getSocketFactory(context);
            socket = (SSLSocket) factory.createSocket();
            socket.connect(new InetSocketAddress(ip, port), CONNECT_TIMEOUT);

            // socket.setEnabledProtocols(new String[] {"TLSv1.2"});

            socket.setKeepAlive(true);
            socket.setSoTimeout(2000); // Timeout de lecture

            inputStream = socket.getInputStream();
            outputStream = socket.getOutputStream();
            connected.set(true);

            // Affiche la suite de chiffrement utilisée
            String cipherSuite = socket.getSession().getCipherSuite();
            Log.d(TAG, "SSL connection established with cipher: " + cipherSuite);

            Log.d(TAG, "Connected successfully");
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Connection failed: " + e.getMessage(), e);
            connected.set(false);
            return false;
        }
    }

    /**
     * Lit des données depuis la connexion.
     * @param buffer Le buffer de réception.
     * @param maxLength La taille maximale à lire.
     * @return Le nombre d'octets lus, 0 si timeout, -1 en cas d'erreur.
     */
    public int read(byte[] buffer, int maxLength) {
        if (!connected.get() || inputStream == null) {
            return -1;
        }

        try {
            return inputStream.read(buffer, 0, maxLength);
        } catch (IOException e) {
            // Gestion du timeout ou des erreurs non bloquantes
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
     * Écrit des données sur la connexion.
     * @param data Les données à envoyer.
     * @param length La longueur des données.
     * @return true si l'envoi a réussi, false sinon.
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
     * Ferme la connexion et libère les ressources.
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
     * Vérifie si la connexion est active.
     * @return true si connecté, false sinon.
     */
    public boolean isConnected() {
        return connected.get() && socket != null && socket.isConnected() && !socket.isClosed();
    }
}