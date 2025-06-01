package com.example.client_android.tcp;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

/**
 * Envoi et réception “raw” (préfixés d’un int longueur big‑endian).
 */
public class Connection {
    private final Socket socket;
    private final DataInputStream in;
    private final DataOutputStream out;

    public Connection(String host, int port) throws IOException {
        this.socket = new Socket(host, port);
        this.in  = new DataInputStream(socket.getInputStream());
        this.out = new DataOutputStream(socket.getOutputStream());
    }

    /** Envoie un byte[] précédé de sa longueur */
    public synchronized void sendRaw(byte[] data) throws IOException {
        out.writeInt(data.length);
        out.write(data);
        out.flush();
    }

    /** Bloquant : lit d’abord un int longueur, puis charge tout le payload */
    public synchronized byte[] receiveRaw() throws IOException {
        int len = in.readInt();
        byte[] buf = new byte[len];
        in.readFully(buf);
        return buf;
    }

    public void close() throws IOException {
        socket.close();
    }
}
