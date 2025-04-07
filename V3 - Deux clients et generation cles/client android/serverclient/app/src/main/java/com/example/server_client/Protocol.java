package com.example.server_client;
import android.util.Log;
public class Protocol {
    private static final String TAG = "Protocol";
    public static final String CMD_X = "CMD_X";
    public static final String CMD_Y = "CMD_Y";

    public byte[] encodeMessage(String cmd, String param) {
        String message = cmd + "|" + param;
        Log.d(TAG, "Encoded message: " + message);
        return (message + "\0").getBytes(); // Add null terminator like C code
    }

    public String[] decodeMessage(String message) {
        String[] parts = new String[]{"", ""};
        int separatorIndex = message.indexOf('|');

        if (separatorIndex != -1) {
            parts[0] = message.substring(0, separatorIndex); // Command
            parts[1] = message.substring(separatorIndex + 1); // Parameter
        } else {
            parts[0] = message;
        }

        return parts;
    }
}