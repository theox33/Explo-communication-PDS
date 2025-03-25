# Android Client - Communication avec un Serveur

Ce projet est une application Android permettant à un client de se connecter à un serveur distant, d'envoyer des messages et de recevoir des réponses.

## Structure du Projet

Le projet est composé de plusieurs fichiers Java qui assurent la connexion réseau, la gestion du protocole de communication et l'interface utilisateur.

### 1. `MainActivity.java`
**Rôle :**  
- Gère l'interface utilisateur (UI) et les interactions avec l'utilisateur.
- Permet à l'utilisateur de se connecter/déconnecter du serveur.
- Envoie des messages au serveur et affiche les réponses reçues.

**Principales fonctionnalités :**  
- Initialisation des éléments de l'UI (`EditText`, `Button`, `TextView`).
- Connexion et déconnexion du serveur via `Client`.
- Envoi de messages et affichage des réponses du serveur.
- Utilisation d'un `Handler` pour mettre à jour l'UI depuis un thread de fond.

---

### 2. `Client.java`
**Rôle :**  
- Gère l'ensemble du processus de communication avec le serveur.
- Encapsule les classes `Connection`, `Protocol` et `Communication`.

**Principales fonctionnalités :**  
- Se connecte au serveur via `Connection`.
- Envoie des messages en utilisant `Communication`.
- Reçoit et traite les réponses du serveur.
- Notifie l'`Activity` lorsqu'un message est reçu.

---

### 3. `Connection.java`
**Rôle :**  
- Gère la connexion TCP avec le serveur.

**Principales fonctionnalités :**  
- Se connecte à l'adresse IP et au port du serveur.
- Envoie et reçoit des données via `InputStream` et `OutputStream`.
- Gère les erreurs de connexion et les fermetures de socket.
- Vérifie si la connexion est toujours active.

---

### 4. `Protocol.java`
**Rôle :**  
- Définit le protocole de communication entre le client et le serveur.

**Principales fonctionnalités :**  
- Encode les messages en respectant un format spécifique (`CMD|param`).
- Décode les messages reçus en extrayant la commande et son paramètre.
- Ajoute un caractère de terminaison (`\0`) pour assurer la compatibilité avec des serveurs C.

---

### 5. `Communication.java`
**Rôle :**  
- Gère la communication continue avec le serveur en utilisant un thread dédié.

**Principales fonctionnalités :**  
- Envoie des commandes (`CMD_X` et `CMD_Y`) au serveur.
- Écoute en permanence les messages entrants.
- Détecte les pertes de connexion et gère les erreurs de lecture.
- Informe `Client` des messages reçus.

---

## Fonctionnalités de l'application
1. **Connexion au serveur** : L'utilisateur peut se connecter/déconnecter.
2. **Envoi de messages** : L'utilisateur envoie un message au serveur.
3. **Réception de messages** : L'application affiche les messages du serveur.
4. **Gestion d'erreurs** : L'application gère les pertes de connexion et les échecs d'envoi.

## Configuration du serveur
- **Adresse IP** : `172.23.3.21` (modifiable dans `Client.java`).
- **Port** : `5001`.

## Améliorations possibles
- Ajouter une gestion avancée des erreurs.
- Implémenter une interface plus interactive.
- Supporter d'autres types de messages (JSON, XML, etc.).

---

📌 **Auteur** : Théo et Alexis  
📅 **Date** : 25/03/2025  
📜 **Licence** : Libre d'utilisation et de modification.

