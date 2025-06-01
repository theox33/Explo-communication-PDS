# V2 - Chiffrement TLS – client Android

## Présentation du sous-projet

Ce sous-projet constitue la contre-partie mobile du projet global **« V2 – Chiffrement TLS »** :  
il illustre comment un **appareil Android** peut établir une connexion **SSL/TLS** vers un serveur TCP écrit en C (voir le sous-projet *serveur Raspberry Pi*).

L’application :

* crée un **socket TLS** via un `SSLSocketFactory` personnalisé (trust-store embarqué) ;
* encode / décode les messages au moyen d’un **protocole texte minimaliste** (`CMD|PARAM`) ;
* gère l’I/O réseau dans un **thread dédié** afin de ne jamais bloquer l’UI ;
* affiche en temps réel les échanges et permet d’envoyer des commandes au serveur.

> **API minimale**
>
> ```text
> CMD_X|<param>   → action X côté serveur
> CMD_Y|<param>   → action Y côté serveur
> ```



---
## Fonctionnalités clés

| Module / classe      | Rôle                                                                                      |
|----------------------|-------------------------------------------------------------------------------------------|
| [SSLUtil.java](classcom_1_1example_1_1server__client_1_1SSLUtil.html)     | Charge le certificat auto-signé (`assets/server.crt`) dans un `KeyStore`, puis crée un `SSLSocketFactory` dédié. |
| [Connection.java](classcom_1_1example_1_1server__client_1_1Connection.html)  | Établit la socket TLS, lit / écrit des octets avec time-out et logs détaillés.            |
| [Protocol.java](classcom_1_1example_1_1server__client_1_1Protocol.html)    | Encode et décode les messages (`cmd\|param\0`).                                           |
| [Communication.java](classcom_1_1example_1_1server__client_1_1Communication.html) | Thread de réception, buffering, découpage, appel d’un callback.                        |
| [Client.java](classcom_1_1example_1_1server__client_1_1Client.html)      | Instancie `Connection`, `Protocol`, `Communication`, expose une API simple à l’activité.  |
| [MainActivity.java](classcom_1_1example_1_1server__client_1_1MainActivity.html) | Interface utilisateur : saisie IP + messages, état de la connexion, log des échanges.    |

---

## Architecture répertoire simplifiée

```text
app/
└── src/main/
    ├── java/com/example/server_client/
    │   ├── Client.java
    │   ├── Communication.java
    │   ├── Connection.java
    │   ├── Protocol.java
    │   ├── SSLUtil.java
    │   └── MainActivity.java
    └── res/
        ├── layout/activity_main.xml   # UI principale
        └── values/strings.xml         # Libellés et messages
assets/
└── server.crt                         # Certificat du serveur
```

*Le dossier `assets/` embarque le certificat auto-signé généré par le serveur afin de valider la chaîne TLS côté client.*

---

## Mise en route

### 1. Pré-requis
 
* Sous-projet **serveur C** compilé et lancé sur le Raspberry Pi ou autre appareil (voir sa documentation)

### 2. Compilation et déploiement

À la racine du sous-projet Android depuis Android Studio > Run


### 3. Utilisation de l’appli

1. Lancer le serveur C : `make run-server` sur le Pi.  
2. Ouvrir l’application sur Android.  
3. Saisir l’ **adresse IP** du serveur (port TCP **5001** forcé par défaut).  
4. Appuyer sur **Connect** :  
   * l’UI affiche le chiffrement TLS négocié et envoie un message de test ;  
   * les messages reçus apparaissent dans le log.  
5. Envoyer des textes via le champ *Message* → **Send**.  
6. **Disconnect** pour fermer proprement la socket TLS.

---

## Extensibilité

* **Migration Protobuf** : remplacer `Protocol.java` par la sérialisation Protobuf utilisée côté serveur.  
* **Authentification mutuelle** : intégrer un `KeyManager` côté client pour présenter un certificat au serveur.  

---

## Licence

Distribué sous licence [MIT](https://opensource.org/licenses/MIT), identique au sous-projet serveur.

---