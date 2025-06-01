/**
 * @file TCPClient.java
 * @brief Client TCP pour communication avec serveur C
 * @details Cette classe implémente un client TCP asynchrone permettant
 *          d'envoyer des messages à un serveur TCP et de recevoir des réponses.
 *          La communication s'effectue en arrière-plan pour éviter de bloquer l'UI.
 * @author Votre nom
 * @version 1.0
 * @date 2025
 * @package com.example.v0_explocomm
 */

package com.example.v0_explocomm

import java.io.*;
import java.net.*;
import android.os.AsyncTask;
import android.util.Log;

/**
 * @class TCPClient
 * @brief Client TCP asynchrone pour communication réseau
 * @details Cette classe encapsule la logique de communication TCP avec un serveur distant.
 *          Elle utilise AsyncTask pour effectuer les opérations réseau en arrière-plan
 *          et notifie les résultats via un système de callbacks.
 *
 * @note Configuration réseau requise :
 *       - Permission INTERNET dans AndroidManifest.xml
 *       - Serveur accessible sur le réseau
 *       - Port 8080 ouvert
 *
 * @warning La communication s'effectue en texte clair (non chiffrée)
 */
public class TCPClient {

    /**
     * @var TAG
     * @brief Tag pour les logs Android
     * @details Identifiant utilisé pour filtrer les logs de cette classe
     *          dans le système de logging Android (LogCat)
     */
    private static final String TAG = "TCPClient";

    /**
     * @var SERVER_IP
     * @brief Adresse IP du serveur TCP
     * @details Adresse IPv4 du serveur C à contacter.
     *          Doit être modifiée selon l'adresse réelle du serveur.
     *
     * @warning Remplacez cette IP par l'adresse réelle de votre serveur
     * @note Pour tests locaux, utilisez l'IP de votre machine sur le réseau local
     */
    private static final String SERVER_IP = "192.168.1.100";

    /**
     * @var SERVER_PORT
     * @brief Port TCP du serveur
     * @details Port sur lequel le serveur TCP écoute les connexions entrantes.
     *          Doit correspondre au port configuré côté serveur C.
     */
    private static final int SERVER_PORT = 8080;

    /**
     * @interface TCPClientListener
     * @brief Interface de callback pour les événements de communication
     * @details Cette interface définit les méthodes de callback appelées
     *          pour notifier les résultats des opérations réseau asynchrones.
     *
     * @note L'implémentation de cette interface permet de recevoir :
     *       - Les messages de réponse du serveur
     *       - Les notifications d'erreur de communication
     */
    public interface TCPClientListener {
        /**
         * @brief Callback appelé lors de la réception d'un message du serveur
         * @details Cette méthode est invoquée lorsque le serveur TCP renvoie
         *          une réponse suite à l'envoi d'un message.
         *
         * @param message Réponse textuelle reçue du serveur
         *
         * @note Cette méthode peut être appelée depuis un thread de background
         * @warning L'implémentation doit gérer la mise à jour de l'UI via runOnUiThread()
         */
        void onMessageReceived(String message);

        /**
         * @brief Callback appelé en cas d'erreur de communication
         * @details Cette méthode est invoquée lorsqu'une erreur survient
         *          pendant la communication réseau (connexion, timeout, etc.).
         *
         * @param error Message d'erreur décrivant le problème rencontré
         *
         * @note Cette méthode peut être appelée depuis un thread de background
         * @warning L'implémentation doit gérer la mise à jour de l'UI via runOnUiThread()
         */
        void onError(String error);
    }

    /**
     * @var listener
     * @brief Instance du listener pour les callbacks de communication
     * @details Référence vers l'objet implémentant TCPClientListener
     *          qui recevra les notifications d'événements réseau
     */
    private TCPClientListener listener;

    /**
     * @brief Constructeur du client TCP
     * @details Initialise le client TCP avec un listener pour recevoir
     *          les callbacks d'événements de communication réseau.
     *
     * @param listener Objet implémentant TCPClientListener pour recevoir les callbacks
     *
     * @note Le listener ne peut pas être null
     * @see TCPClientListener
     */
    public TCPClient(TCPClientListener listener) {
        this.listener = listener;
    }

    /**
     * @brief Envoie un message au serveur TCP de manière asynchrone
     * @details Cette méthode lance une tâche asynchrone pour envoyer
     *          le message au serveur sans bloquer le thread principal.
     *          Le résultat sera notifié via les callbacks du listener.
     *
     * @param message Message texte à envoyer au serveur
     *
     * @note La méthode retourne immédiatement, l'envoi s'effectue en arrière-plan
     * @warning Le message ne doit pas être null ou vide
     *
     * @see SendMessageTask
     */
    public void sendMessage(String message) {
        new SendMessageTask().execute(message);
    }

    /**
     * @class SendMessageTask
     * @brief Tâche asynchrone pour l'envoi de messages TCP
     * @details Cette classe interne hérite d'AsyncTask et gère la communication
     *          TCP en arrière-plan. Elle établit la connexion, envoie le message,
     *          reçoit la réponse et nettoie les ressources.
     *
     * @extends AsyncTask<String, Void, String>
     *          - Paramètre d'entrée : String (message à envoyer)
     *          - Paramètre de progression : Void (pas de progression)
     *          - Résultat : String (réponse du serveur)
     *
     * @note Cette classe s'exécute en arrière-plan pour éviter de bloquer l'UI
     * @warning Gère automatiquement la fermeture des ressources réseau
     */
    private class SendMessageTask extends AsyncTask<String, Void, String> {

        /**
         * @var errorMessage
         * @brief Message d'erreur capturé pendant la communication
         * @details Variable utilisée pour stocker les messages d'erreur
         *          survenant pendant les opérations réseau afin de les
         *          transmettre au callback onError()
         */
        private String errorMessage = null;

        /**
         * @brief Méthode principale exécutée en arrière-plan
         * @details Cette méthode effectue toute la logique de communication TCP :
         *          connexion au serveur, envoi du message, réception de la réponse
         *          et nettoyage des ressources. Elle s'exécute sur un thread de background.
         *
         * @param messages Tableau contenant le message à envoyer (messages[0])
         * @return String Réponse du serveur ou null en cas d'erreur
         *
         * @note Gestion automatique des ressources avec try-with-resources simulé
         * @warning Capture toutes les IOException et les transmet via errorMessage
         *
         * @see java.net.Socket
         * @see java.io.PrintWriter
         * @see java.io.BufferedReader
         */
        @Override
        protected String doInBackground(String... messages) {
            Socket socket = null;
            PrintWriter out = null;
            BufferedReader in = null;

            try {
                /**
                 * @brief Établissement de la connexion TCP
                 * @details Crée un socket TCP et se connecte au serveur
                 *          en utilisant l'adresse IP et le port configurés
                 */
                socket = new Socket(SERVER_IP, SERVER_PORT);

                /**
                 * @brief Configuration des flux de communication
                 * @details Initialise les flux d'entrée et de sortie pour
                 *          la communication bidirectionnelle avec le serveur :
                 *          - PrintWriter pour l'envoi (autoflush activé)
                 *          - BufferedReader pour la réception
                 */
                out = new PrintWriter(socket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                /**
                 * @brief Envoi du message au serveur
                 * @details Récupère le premier message du tableau de paramètres
                 *          et l'envoie au serveur via PrintWriter
                 */
                String messageToSend = messages[0];
                out.println(messageToSend);
                Log.d(TAG, "Message envoyé: " + messageToSend);

                /**
                 * @brief Réception de la réponse du serveur
                 * @details Lit la réponse du serveur via BufferedReader
                 *          et l'enregistre dans les logs pour debug
                 */
                String response = in.readLine();
                Log.d(TAG, "Réponse reçue: " + response);

                return response;

            } catch (IOException e) {
                /**
                 * @brief Gestion des erreurs de communication
                 * @details Capture les exceptions IOException, enregistre
                 *          l'erreur dans errorMessage et retourne null
                 */
                errorMessage = "Erreur de connexion: " + e.getMessage();
                Log.e(TAG, errorMessage, e);
                return null;
            } finally {
                /**
                 * @brief Nettoyage des ressources réseau
                 * @details Ferme proprement toutes les ressources ouvertes
                 *          dans l'ordre inverse de leur ouverture pour éviter
                 *          les fuites de ressources
                 */
                try {
                    if (out != null) out.close();
                    if (in != null) in.close();
                    if (socket != null) socket.close();
                } catch (IOException e) {
                    Log.e(TAG, "Erreur lors de la fermeture", e);
                }
            }
        }

        /**
         * @brief Méthode appelée après l'exécution en arrière-plan
         * @details Cette méthode s'exécute sur le thread principal après
         *          la fin de doInBackground(). Elle transmet le résultat
         *          via les callbacks du listener approprié.
         *
         * @param result Résultat retourné par doInBackground() (réponse du serveur ou null)
         *
         * @note S'exécute sur le thread principal, peut modifier l'UI directement
         * @warning Vérifie la nullité du listener avant d'appeler les callbacks
         *
         * @see TCPClientListener#onMessageReceived(String)
         * @see TCPClientListener#onError(String)
         */
        @Override
        protected void onPostExecute(String result) {
            if (result != null && listener != null) {
                /**
                 * @brief Notification de succès
                 * @details Appelle le callback onMessageReceived() avec la réponse du serveur
                 */
                listener.onMessageReceived(result);
            } else if (errorMessage != null && listener != null) {
                /**
                 * @brief Notification d'erreur
                 * @details Appelle le callback onError() avec le message d'erreur capturé
                 */
                listener.onError(errorMessage);
            }
        }
    }
}