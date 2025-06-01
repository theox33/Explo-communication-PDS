
# Exploration technique – Serveur C & Client Android : Sérialisation Protocol Buffers

**Auteur :** Théo AVRIL

---

## 1 · Objectif de l’exploration

Montrer qu’un **serveur TCP écrit en C** et une **application Android (Java)** peuvent échanger de manière fiable des messages binaires encodés avec **Protocol Buffers v1 (Protobuf)**, puis mettre en évidence les principaux avantages fonctionnels et techniques d'un tel encodage.

---

## 2 · État de l’art

### 2.1 · Bibliothèques Protobuf côté C

| Lib | Atouts majeurs | Limites |
|-----|----------------|---------|
| **protobuf-c** | Génère du C pur, empreinte ≈ 200 Ko, utilisation directe de `malloc`/`free`, disponible dans les dépôts Debian | API simple mais manuelle ; pas de RPC intégré |
| **FlatBuffers 24.x** | Zéro-copy, lecture directe en mémoire | Format plus verbeux que Protobuf, outillage C plus complexe |

> **Constat :** pour un Raspberry Pi, **protobuf-c** représente le meilleur compromis « fonctionnalités vs taille binaire ». FlatBuffers répond à d’autres besoins (jeux, 3D).

### 2.2 · Protobuf sur Android

*Choix retenu :* `protobuf-javalite` qui s’intègre nativement au plug-in Gradle `com.google.protobuf`.

### 2.3 · Pourquoi Protobuf plutôt que BSON ?

| Critère | **Protobuf** | **BSON** |
|---------|--------------|-----------|
| Taille « Hello »¹ | **≈ 9 octets** | ≈ 41 octets |
| Schéma formel | Oui (`.proto`) | Optionnel |
| Compat. ascendante | Champs inconnus ignorés | Champs inconnus ignorés mais non documentés |
| Code généré type-safe | Oui (getters, setters) | Non (accès par clé chaîne) |

¹ Message : `author="Bob", text="Hi"`

---

## 3 · Choix d’implémentation

| Aspect | Décision | Motivation |
|--------|----------|------------|
| **Lib C** | `protobuf-c` | Package Debian stable ; API directe ; communauté active |
| **Lib Android** | Plugin JetBrains `idea.plugin.protoeditor` | Runtime minimal ; support Google Play |
| **Transport** | TCP (port 12345) | Comparaison directe avec protocole texte |
| **Schéma** | `AMessage { content }` | Suffisant pour démonstration |
| **Sécurité** | *Aucune* (POC) → TLS (exploration V2) | Garde la sérialisation indépendante du chiffrement |

---

## 4 · Description des projets exemples de mise en œuvre

### 4.1 · Sous-projet : Serveur C – Raspberry Pi

* **Rôle :** écouter sur **:12345**, décoder les messages Protobuf reçus, les journaliser puis éventuellement répondre.
* **Arborescence simplifiée :**
  * `client.c` : client CLI de test.
  * `connection.c` : acceptation TCP, thread par client.
  * `package/protocol/protobuf/src/` : `message.proto`, `protocol.{c,h}` (wrap `pack/unpack`).
  * `.../dist/src/` : `message.pb-c.{c,h}` générés par `protoc`.

* **Build :** `make && make run-server` (dépendence `libprotobuf-c`).

```text
.
├── makefile
└── src
    ├── client.c
    ├── connection.c
    └── package
        └── protocol
            └── protobuf
                ├── dist
                │   └── src
                │       ├── message.pb-c.c
                │       └── message.pb-c.h
                ├── Makefile
                └── src
                    ├── message.proto
                    ├── protocol.c
                    └── protocol.h
```

### 4.2 · Sous-projet : Client Android

* **Rôle :** établir la socket TCP, sérialiser/désérialiser les messages Protobuf, afficher le flux dans l’UI.
* **Répertoires clés :**
  * `app/src/main/proto/message.proto` : schéma partagé.
  * `Connection.java` : gestion socket + I/O.
  * `MainActivity.java` : interface utilisateur.
* **Build Gradle** : plug-in `com.google.protobuf` + dépendance `protobuf-javalite`.

---

## 5 · Perspectives

* **TLS** : réutiliser la stack TLS (exploration V2) pour chiffrer le transport.
---

## 6 · Références

1. Medium - *Setting Up Protocol Buffers in an Android Project* (2024) <https://medium.com/@zekromvishwa56789/setting-up-protocol-buffers-in-an-android-project-8f7bad31981f>
2. GitHub - *Protoc* - <https://github.com/protobuf-c/protobuf-c>
3. Linuxembedded - *Introduction de Protobuf en C* <https://linuxembedded.fr/2023/11/introduction-de-protobuf-en-c>

---

> Document sous licence MIT ; le code source complet est disponible dans les répertoires *src/* (C) et *app/* (Android).
