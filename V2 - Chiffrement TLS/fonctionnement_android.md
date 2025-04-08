# Fonctionnement de l'application Android

## 📱 Classes et leur rôle

### 1. `MainActivity`
**Rôle :** Interface utilisateur principale.

**Fonctionnalités :**
- Permet à l'utilisateur de se connecter/déconnecter au serveur via un bouton.
- Permet d'envoyer des messages au serveur.
- Affiche les messages reçus du serveur dans une zone de texte.
- Gère les interactions utilisateur et met à jour l'interface en fonction de l'état de la connexion.

---

### 2. `Client`
**Rôle :** Point d'entrée pour gérer la communication avec le serveur.

**Fonctionnalités :**
- Initialise les composants nécessaires : `Connection`, `Protocol`, et `Communication`.
- Fournit des méthodes pour se connecter au serveur, envoyer des messages, et se déconnecter.
- Gère un écouteur (`MessageListener`) pour traiter les messages reçus du serveur.
- Envoie un message de test après la connexion pour vérifier la communication.

---

### 3. `Connection`
**Rôle :** Gère la connexion réseau sécurisée via TLS.

**Fonctionnalités :**
- Établit une connexion SSL/TLS avec le serveur en utilisant un socket sécurisé.
- Fournit des méthodes pour lire et écrire des données sur le socket.
- Gère les erreurs de connexion et assure la fermeture propre de la connexion.
- Vérifie si la connexion est toujours active.

---

### 4. `Protocol`
**Rôle :** Définit le format des messages échangés entre le client et le serveur.

**Fonctionnalités :**
- Encode les messages au format `CMD|PARAM` (commande + paramètre).
- Décode les messages reçus en séparant commande et paramètre.
- Permet de structurer les messages pour une communication standardisée.

---

### 5. `Communication`
**Rôle :** Gère la logique de communication entre le client et le serveur.

**Fonctionnalités :**
- Utilise `Protocol` pour encoder/décoder les messages.
- Utilise `Connection` pour envoyer et recevoir des données.
- Exécute un thread en arrière-plan pour écouter les messages du serveur en continu.
- Appelle un gestionnaire de messages (`MessageHandler`) lorsqu'un message est reçu.
- Fournit des méthodes pour envoyer des commandes spécifiques (`comX`, `comY`).

---

### 6. `SSLUtil`
**Rôle :** Fournit des utilitaires pour configurer et créer des sockets SSL/TLS.

**Fonctionnalités :**
- Configure les certificats nécessaires pour établir une connexion sécurisée.
- Crée une instance de `SSLSocketFactory` pour établir des connexions sécurisées.

---

## 🔄 Comment fonctionne la communication

### Connexion au serveur :
1. L'utilisateur appuie sur le bouton **"Connect"**.
2. `MainActivity` appelle `Client.connectToServer()`.
3. `Client` utilise `Connection` pour établir une connexion SSL/TLS avec le serveur.
4. Si la connexion est établie, un thread de communication est lancé via `Communication.run()`.

---

### Envoi de messages :
1. L'utilisateur saisit un message et appuie sur **"Send"**.
2. `MainActivity` appelle `Client.sendMessage()`.
3. `Client` utilise `Communication.comX()` pour encoder le message avec `Protocol` et l'envoyer via `Connection`.

---

### Réception de messages :
1. Le thread de communication dans `Communication` écoute les messages du serveur.
2. Lorsqu'un message est reçu, il est décodé avec `Protocol.decodeMessage()`.
3. Le gestionnaire de messages (`MessageHandler`) est appelé pour traiter le message et mettre à jour l'interface utilisateur.

---

### Déconnexion :
1. L'utilisateur appuie sur **"Disconnect"**.
2. `MainActivity` appelle `Client.disconnect()`.
3. `Client` arrête le thread de communication et ferme la connexion via `Connection.close()`.

---

## 🧩 Résumé des interactions

| Composant     | Rôle principal                               |
|---------------|----------------------------------------------|
| MainActivity  | Interface utilisateur                        |
| Client        | Coordonne la communication                   |
| Connection    | Gère la connexion réseau sécurisée           |
| Protocol      | Définit le format des messages               |
| Communication | Gère l'envoi/réception des messages          |
| SSLUtil       | Configure la sécurité SSL/TLS                |

---

✅ Ce système assure une **communication sécurisée** et **structurée** entre le client Android et le serveur.
