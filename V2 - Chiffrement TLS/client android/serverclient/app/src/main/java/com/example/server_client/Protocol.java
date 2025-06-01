package com.example.server_client;
import android.util.Log;

/**
 * @class Protocol
 * @brief Gère l'encodage et le décodage des messages selon un protocole simple.
 * Il ne s'agit pas d'un protocole de communication complet, mais d'une structure de base pour envoyer des commandes et des paramètres.
 */
public class Protocol {
    private static final String TAG = "Protocol";
    public static final String CMD_X = "CMD_X"; // Commande X
    public static final String CMD_Y = "CMD_Y"; // Commande Y

    /**
     * Encode une commande et un paramètre en un message à envoyer.
     * @param cmd La commande à encoder.
     * @param param Le paramètre associé à la commande.
     * @return Le message encodé sous forme de tableau d'octets (avec séparateur et fin de chaîne).
     */
    public byte[] encodeMessage(String cmd, String param) {
        String message = cmd + "|" + param; // Concatène la commande et le paramètre avec un séparateur
        Log.d(TAG, "Encoded message: " + message); // Log pour le debug
        return (message + "\0").getBytes(); // Ajoute un caractère de fin de chaîne
    }

    /**
     * Décode un message reçu en séparant la commande et le paramètre.
     * @param message Le message à décoder.
     * @return Un tableau de chaînes : [commande, paramètre].
     */
    public String[] decodeMessage(String message) {
        String[] parts = new String[]{"", ""}; // Initialise le tableau de retour
        int separatorIndex = message.indexOf('|'); // Cherche le séparateur

        if (separatorIndex != -1) {
            // Si le séparateur existe, découpe la commande et le paramètre
            parts[0] = message.substring(0, separatorIndex);
            parts[1] = message.substring(separatorIndex + 1);
        } else {
            // Sinon, tout le message est considéré comme la commande
            parts[0] = message;
        }

        return parts;
    }
}