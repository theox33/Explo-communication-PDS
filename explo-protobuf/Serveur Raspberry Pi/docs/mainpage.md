
# V1 – Protocol Buffers – serveur Raspberry Pi

**Auteur** : Théo AVRIL

---

## 1 · Présentation du sous‑projet

Ce sous‑projet appartient à l’exploration **« V1 – Protocol Buffers »** ; il démontre qu’un **serveur écrit en C** peut échanger, sur une simple socket TCP, des messages binaires **Protobuf v1** avec :

* un client de test écrit en C (fourni) ;
* une application Android (voir le sous‑projet *client Android*).

Le serveur est pensé pour fonctionner sur un **Raspberry Pi** mais reste portable sur toute distribution GNU/Linux.

---

## 2 · Objectifs techniques

1. **Sérialiser efficacement** les messages définis dans `message.proto`.  
2. **Valider l’interopérabilité** C / Java grâce aux stubs générés par `protobuf‑c` et `protobuf‑javalite`.

---

## 3 · Structure du code

| Fichier / Module | Rôle |
|------------------|------|
| `connection.c` | Point d’entrée : ouvre le port **12345**, accepte les clients, délègue la (dé)sérialisation. |
| `package/protocol/protobuf/src/message.proto` | Schéma Protobuf commun (`AMessage { content }`). |
| `[package/protocol/protobuf/src/protocol.c\h](protocol_8h.html)` | Enveloppe utilitaire : `protocol_encrypt_message`, `protocol_decrypt_message`. |
| `package/protocol/protobuf/dist/src/message.pb-c.[c\|h]` | **Fichiers générés** par `protoc --c_out`, à ne pas modifier. |
| `src/client.c` | Client CLI pour tester localement (envoi d’un texte). |

---

## 4 · Architecture répertoire simplifiée

```text
.
├── makefile
└── src/
    ├── client.c
    ├── connection.c
    └── package/protocol/protobuf/
        ├── src/
        │   ├── message.proto
        │   ├── protocol.c
        │   └── protocol.h
        └── dist/src/
            ├── message.pb-c.c
            └── message.pb-c.h
```

---

## 5 · Utilisation

### 5.1 · Compilation & lancement serveur

```sh
# Depuis la racine du sous-projet
make generate   # compile les stubs, la lib protobuf-c et le serveur
make run-server # écoute sur 0.0.0.0:12345
```

### 5.2 · Test avec le client C

Dans un second terminal :

```sh
make run-client
# saisir un texte ⇒ affichage côté serveur puis retour écho
```

### 5.3 · Test avec le client Android

1. Compiler et lancer l’application (voir documentation Android).  
2. Appuyer sur **Connect** puis **Send** ; le serveur journalise chaque `AMessage`.

---

## 6 · Perspectives

* **TLS** : intégrer OpenSSL (exploration V2) pour chiffrer la socket.  

---

## 7 · Licence

Code et documentation sous licence [MIT](https://opensource.org/licenses/MIT).

---
