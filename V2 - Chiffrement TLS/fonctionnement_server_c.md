# 📡 Fonctionnement du Serveur et de la Communication Client (Android) / Serveur (C)

## 🔄 Communication entre client Android et serveur C

### 🧱 Initialisation du serveur
- Le fichier `server.c` initialise un **contexte TLS** avec OpenSSL et génère dynamiquement des clés SSL.
- Il **écoute les connexions entrantes** sur le port `5001`.
- Lorsqu’un client se connecte, un **thread dédié** est créé pour gérer cette connexion.

---

### 🔐 Connexion du client
- Le client Android (`Client.java`) établit une connexion sécurisée via `Connection` et `SSLUtil`.
- Une fois connecté, un **thread de communication** est lancé (`Communication.java`) pour l’échange de messages.

---

### ✉️ Échange de messages
- Les messages sont encodés/décodés avec la classe `Protocol` (commune au client et serveur).
- Le client envoie des commandes (`CMD_X`, `CMD_Y`) accompagnées de paramètres.
- Le serveur traite ces commandes et peut **diffuser des réponses à d’autres clients connectés**.

---

### 🧵 Gestion des threads
- Le client et le serveur utilisent des **threads asynchrones** pour la communication.
- Le serveur appelle une fonction `message_handler` pour traiter les messages reçus et répondre.

---

### ❌ Déconnexion
- Lorsqu'une déconnexion est initiée, les **ressources (sockets, threads)** sont correctement libérées côté client et serveur.

---

## 🧩 Rôle de chaque composant

### 📱 Côté Android

#### 1. `MainActivity`
**But :** Interface utilisateur du client Android.  
**Responsabilités :**
- Connecter/déconnecter au serveur.
- Envoyer des messages.
- Afficher les messages reçus.

#### 2. `Client`
**But :** Gérer toute la logique de communication côté client.  
**Responsabilités :**
- Établir une connexion TLS via `Connection`.
- Utiliser `Communication` et `Protocol` pour les échanges.
- Notifier l’interface via `MessageListener`.

#### 3. `Connection`
**But :** Gérer les connexions bas-niveau avec TLS.  
**Responsabilités :**
- Utiliser `SSLUtil` pour établir une socket sécurisée.
- Lire/écrire sur la socket.
- Gérer l’état de la connexion.

#### 4. `Protocol`
**But :** Définir le **format standardisé** des messages (`CMD|PARAM`).  
**Responsabilités :**
- Encoder et décoder les messages.

#### 5. `Communication`
**But :** Gérer la logique de communication haut-niveau.  
**Responsabilités :**
- Utiliser `Protocol` + `Connection`.
- Lancer un thread d’écoute.
- Appeler un gestionnaire de messages.

#### 6. `SSLUtil`
**But :** Créer des sockets sécurisées côté Android.  
**Responsabilités :**
- Charger les certificats.
- Créer une `SSLSocketFactory`.

---

### 💻 Côté Serveur (C sur Raspberry Pi)

#### 7. `server.c`
**But :** Logique principale serveur multi-clients.  
**Responsabilités :**
- Initialiser TLS.
- Écouter sur un port.
- Créer un thread par client.
- Utiliser `message_handler`.

#### 8. `connection.c`
**But :** Gérer la couche réseau serveur.  
**Responsabilités :**
- Créer des sockets.
- Lire/écrire les données.
- Gérer les erreurs.

#### 9. `protocol.c`
**But :** Formatage des messages côté serveur.  
**Responsabilités :**
- Encodage `CMD|PARAM`.
- Décodage des messages reçus.

#### 10. `communication.c`
**But :** Communication serveur-client.  
**Responsabilités :**
- Utiliser `protocol.c` + `connection.c`.
- Gérer un thread par client.
- Appeler le `message_handler`.

---

## 🔁 Résumé des interactions

| Élément | Client (Android) | Serveur (Raspberry Pi - C) |
|--------|-------------------|-----------------------------|
| Interface | `MainActivity` | `server.c` |
| Connexion TLS | `Connection`, `SSLUtil` | `server.c`, `OpenSSL` |
| Format des messages | `Protocol` | `protocol.c` |
| Communication | `Communication` | `communication.c` |
| Gestion socket | `Connection` | `connection.c` |
| Thread de traitement | `Communication` | `message_handler` |

---

✅ Cette architecture assure une **communication sécurisée, modulaire et performante** entre un client Android et un serveur C embarqué sur Raspberry Pi.
