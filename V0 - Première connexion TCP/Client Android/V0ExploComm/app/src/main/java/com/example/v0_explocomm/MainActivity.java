/**
 * @file MainActivity.java
 * @brief Activité principale de l'application cliente TCP Android
 * @details Cette classe implémente l'interface utilisateur principale permettant
 *          d'envoyer des messages à un serveur TCP en C et d'afficher les réponses.
 *          Elle gère l'interaction utilisateur et la communication réseau asynchrone.
 * @author Votre nom
 * @version 1.0
 * @date 2025
 * @package com.example.v0_explocomm
 */

package com.example.v0_explocomm;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

/**
 * @class MainActivity
 * @brief Activité principale gérant l'interface utilisateur et la communication TCP
 * @details Cette classe hérite d'AppCompatActivity et implémente TCPClient.TCPClientListener
 *          pour gérer les callbacks de communication réseau. Elle fournit une interface
 *          simple pour envoyer des messages et afficher les réponses du serveur.
 *
 * @implements TCPClient.TCPClientListener Interface pour recevoir les callbacks réseau
 * @extends AppCompatActivity Classe de base pour les activités Android compatibles
 */
public class MainActivity extends AppCompatActivity implements TCPClient.TCPClientListener {

    /**
     * @var editTextMessage
     * @brief Champ de saisie pour le message à envoyer
     * @details Widget EditText permettant à l'utilisateur de taper le message
     *          à transmettre au serveur TCP
     */
    private EditText editTextMessage;

    /**
     * @var buttonSend
     * @brief Bouton pour déclencher l'envoi du message
     * @details Widget Button qui déclenche la transmission du message
     *          vers le serveur lorsqu'il est pressé
     */
    private Button buttonSend;

    /**
     * @var textViewResponse
     * @brief Zone d'affichage des réponses du serveur
     * @details Widget TextView qui affiche les réponses reçues du serveur
     *          ou les messages d'erreur en cas de problème de connexion
     */
    private TextView textViewResponse;

    /**
     * @var tcpClient
     * @brief Instance du client TCP pour la communication réseau
     * @details Objet TCPClient gérant la communication asynchrone avec le serveur.
     *          Utilise des callbacks pour notifier les résultats des opérations.
     */
    private TCPClient tcpClient;

    /**
     * @brief Méthode appelée lors de la création de l'activité
     * @details Initialise l'interface utilisateur, configure les widgets
     *          et met en place les listeners d'événements. Cette méthode
     *          est automatiquement appelée par le système Android.
     *
     * @param savedInstanceState État sauvegardé de l'activité (peut être null)
     *
     * @note Cette méthode configure :
     *       - Le layout de l'activité
     *       - Les références aux widgets UI
     *       - L'instance du client TCP
     *       - Le listener du bouton d'envoi
     *
     * @see TCPClient#TCPClient(TCPClient.TCPClientListener)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /**
         * @brief Initialisation des références aux widgets UI
         * @details Récupère les références des éléments d'interface définis
         *          dans le fichier layout XML via leurs identifiants
         */
        editTextMessage = findViewById(R.id.editTextMessage);
        buttonSend = findViewById(R.id.buttonSend);
        textViewResponse = findViewById(R.id.textViewResponse);

        /**
         * @brief Création de l'instance du client TCP
         * @details Instancie le client TCP en passant cette activité comme listener
         *          pour recevoir les callbacks de communication réseau
         */
        tcpClient = new TCPClient(this);

        /**
         * @brief Configuration du listener du bouton d'envoi
         * @details Définit l'action à effectuer lorsque l'utilisateur appuie
         *          sur le bouton d'envoi : validation et transmission du message
         */
        buttonSend.setOnClickListener(new View.OnClickListener() {
            /**
             * @brief Gestionnaire de clic du bouton d'envoi
             * @details Méthode appelée lorsque l'utilisateur appuie sur le bouton.
             *          Valide le contenu du champ de saisie et déclenche l'envoi
             *          du message via le client TCP.
             *
             * @param v Vue du bouton qui a été cliqué
             *
             * @note Comportement :
             *       - Récupère et nettoie le texte saisi
             *       - Vérifie que le message n'est pas vide
             *       - Envoie le message via TCPClient
             *       - Vide le champ de saisie
             *       - Affiche un toast d'erreur si le message est vide
             */
            @Override
            public void onClick(View v) {
                String message = editTextMessage.getText().toString().trim();
                if (!message.isEmpty()) {
                    tcpClient.sendMessage(message);
                    editTextMessage.setText("");
                } else {
                    Toast.makeText(MainActivity.this, "Veuillez saisir un message", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    /**
     * @brief Callback appelé lors de la réception d'un message du serveur
     * @details Cette méthode est appelée par TCPClient lorsqu'une réponse
     *          est reçue du serveur TCP. Elle met à jour l'interface utilisateur
     *          pour afficher la réponse reçue.
     *
     * @param message Message reçu du serveur TCP
     *
     * @note Cette méthode s'exécute sur un thread de background, d'où l'utilisation
     *       de runOnUiThread() pour modifier l'interface utilisateur
     *
     * @warning Toute modification d'UI doit être effectuée sur le thread principal
     *
     * @see TCPClient.TCPClientListener#onMessageReceived(String)
     */
    @Override
    public void onMessageReceived(String message) {
        /**
         * @brief Mise à jour de l'UI sur le thread principal
         * @details Utilise runOnUiThread() pour s'assurer que les modifications
         *          de l'interface utilisateur s'effectuent sur le thread principal
         */
        runOnUiThread(new Runnable() {
            /**
             * @brief Exécution sur le thread UI principal
             * @details Met à jour le TextView de réponse et affiche un toast
             *          de confirmation de réception du message
             */
            @Override
            public void run() {
                textViewResponse.setText("Réponse du serveur: " + message);
                Toast.makeText(MainActivity.this, "Message reçu!", Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * @brief Callback appelé en cas d'erreur de communication
     * @details Cette méthode est appelée par TCPClient lorsqu'une erreur
     *          survient pendant la communication réseau (connexion échouée,
     *          timeout, etc.). Elle affiche l'erreur à l'utilisateur.
     *
     * @param error Message d'erreur décrivant le problème rencontré
     *
     * @note Cette méthode s'exécute sur un thread de background, d'où l'utilisation
     *       de runOnUiThread() pour modifier l'interface utilisateur
     *
     * @warning Les erreurs courantes incluent :
     *          - Serveur non accessible
     *          - Problème de réseau
     *          - Permissions manquantes
     *
     * @see TCPClient.TCPClientListener#onError(String)
     */
    @Override
    public void onError(String error) {
        /**
         * @brief Affichage de l'erreur sur le thread principal
         * @details Utilise runOnUiThread() pour s'assurer que les modifications
         *          de l'interface utilisateur s'effectuent sur le thread principal
         */
        runOnUiThread(new Runnable() {
            /**
             * @brief Exécution sur le thread UI principal
             * @details Met à jour le TextView de réponse avec le message d'erreur
             *          et affiche un toast d'erreur prolongé pour informer l'utilisateur
             */
            @Override
            public void run() {
                textViewResponse.setText("Erreur: " + error);
                Toast.makeText(MainActivity.this, error, Toast.LENGTH_LONG).show();
            }
        });
    }
}