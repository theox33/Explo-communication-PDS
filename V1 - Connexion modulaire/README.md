# 📡 Application de Communication TCP – Serveur Raspberry Pi & Client Android

Ce projet démontre un système simple de **communication via TCP** entre un **serveur en C** tournant sur un **Raspberry Pi** et un **client Android en Java** fonctionnant sur une tablette ou un téléphone.

Actuellement, le système permet :
- À un seul client Android d’envoyer des messages au serveur Raspberry Pi via une connexion TCP.

---

## 🚀 Présentation du Projet

- **Serveur** : Écrit en C, conçu pour fonctionner sur un Raspberry Pi.
- **Client** : Application Android écrite en Java.
- **Protocole de communication** : TCP (pas encore de chiffrement ni de TLS).
- **Architecture actuelle** : Un seul client peut se connecter et envoyer des messages. Le serveur les reçoit mais ne répond pas encore.

---

## 🧠 Objectifs & Feuille de Route

### ✅ Fonctionnalités actuelles (v1)
- [x] Le serveur écoute sur un port spécifique les connexions TCP entrantes.
- [x] Le client Android établit une connexion TCP et envoie des messages.
- [x] Le serveur Raspberry Pi reçoit les messages.

### 🔜 Fonctionnalités prévues (v2 et suivantes)
- [ ] Le serveur répond aux messages du client.
- [ ] Support de plusieurs clients (au moins 2).
- [ ] Communication **entre clients** via le serveur (système de relais).
- [ ] Ajout d’un chiffrement (TLS/SSL) pour sécuriser les échanges.
- [ ] Amélioration de la gestion des erreurs et du retour utilisateur.

---

## 🧰 Technologies Utilisées

| Composant | Langage/Outil        |
|-----------|----------------------|
| Serveur   | C (Sockets POSIX)    |
| Client    | Java (SDK Android)   |
| Protocole | TCP                  |

---