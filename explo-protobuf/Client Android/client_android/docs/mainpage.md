
# V1 – Protobuf – client Android

## Présentation du sous-projet

Ce sous-projet constitue la contre‑partie mobile du projet global **« V1 – Protocol Buffers »** :  
il montre comment une **application Android** peut échanger des messages **Protobuf** avec un serveur TCP écrit en C (voir le sous‑projet *serveur Raspberry Pi*).

L’application :

* établit une **socket TCP** directe ;
* sérialise et désérialise les messages grâce à `protobuf` générés depuis `message.proto` ;
* applique un **chiffrement XOR** minimal avant l’envoi / après la réception (POC avant TLS) ;
* gère l’I/O réseau dans un **thread dédié** afin de ne jamais bloquer l’UI ;
* affiche en temps réel les échanges et permet d’envoyer des textes.

**Format d’échange**

Chaque message est un `AMessage` défini dans `message.proto` :  
`string content`  


---

## Fonctionnalités clés

| Module / classe | Rôle |
|-----------------|------|
| `Connection.java` | Activité principale : ouvre / ferme la socket, lance le thread de réception, orchestre l’UI. |
| `Message.java` (généré) | Stubs `protobuf‑javalite` pour le type `AMessage`. |
| `activity_main.xml` | Interface utilisateur : zone de texte, boutons **Connect / Send** . |

---

## Architecture répertoire simplifiée

```text
app/
└── src/main/
    ├── java/com/example/client_android/
    │   └── Connection.java
    ├── proto/message.proto          # schéma partagé
    └── res/
        ├── layout/activity_main.xml # UI principale
        └── values/strings.xml       # Libellés
```

---

## Mise en route

### 1. Pré‑requis

* Sous‑projet **serveur C** compilé et lancé sur le Raspberry Pi ou autre machine (port **12345** par défaut).
* Android Studio.

### 2. Compilation et déploiement

1. Ouvrir le dossier **app/** dans Android Studio.   
2. Lancer *Run* : l’APK est déployé sur l’émulateur ou un appareil physique (Debug).

### 3. Utilisation de l’application

1. Appuyer sur **Connect** :  
   * la socket TCP s’ouvre ;  
   * le bouton **Send** devient actif ;  
   * un log *[Connected to Hermes]* apparaît.  
2. Saisir un texte → **Send** :  
   * le client sérialise le message et l’envoie ;  
   * la zone de log affiche `Ares: <texte>` puis, à la réception, `Hermes: <texte>`.  
3. **Disconnect** pour fermer proprement la socket.

---

## Extensibilité

* **TLS** : ajout d'une sécurité TLS (voir explo V2)

---

## Licence

Distribué sous licence [MIT](https://opensource.org/licenses/MIT).

---
