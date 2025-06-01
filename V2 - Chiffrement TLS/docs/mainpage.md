
# Exploration technique – Serveur C & Client Android : Connexion TLS sécurisée

**Date :** 1 juin 2025  
**Auteur :** Théo AVRIL

---

## 1 · Objectif de l’exploration

Démontrer, à l’aide d’un **serveur écrit en C** (hébergé sur Raspberry Pi) et d’un **client Android en Java**, qu’il est possible d’établir une **connexion TCP chiffrée par TLS/SSL** inter-plateformes ; fournir un socle reproductible et extensible (mutual TLS, Protobuf, etc.).

---

## 2 · État de l’art

### 2.1 · Bibliothèques TLS côté C

| Lib | Licence | Atouts majeurs | Limites |
|-----|---------|----------------|---------|
| **OpenSSL 3.x** | Apache Licence 2.0 | Référence historique, large écosystème, support TLS 1.3, compatibilité UNIX/Windows, accélération matérielle | API réputée complexe ; empreinte mémoire importante ; thread-safety partielle nécessitant précautions [1][2] |
| **wolfSSL 5.x** | GPLv2/commercial | Empreinte ~100–200 KiB, optimisations embarquées, performances supérieures (x2 connexions/s vs OpenSSL 1.1.1) [3] | API spécifique, licence GPL payante pour produit fermé |
| **MbedTLS (ARM)** | Apache 2.0 | Conçu pour micro-contrôleurs, code lisible, dépendances minimales [4] | Fonctionnalités moins riches (pas d’ALG EC J-PAKE, perf ≤ OpenSSL) |

**Choix retenu** : **OpenSSL** – disponibilité native sur Raspberry Pi, documentation abondante, compatibilité avec les bibliothèques Java (BoringSSL/Conscrypt) utilisées sur Android.

---

### 2.2 · Pile TLS sur Android

| Composant | Depuis | Rôle / remarques |
|-----------|--------|------------------|
| **BoringSSL** (fork Google d’OpenSSL) | Android 5.0 | Implémente TLS 1.2/1.3 ; optimisations ARM NEON |
| **SSLSocket / SSLSocketFactory** | API 1 | API bas niveau Javax permettant la création de sockets TLS [5] |
| **Conscrypt Provider** | ext. | Back-port de TLS 1.3 (< API 29) [6] |
| **Network Security Config** | Android 7.0 | XML déclaratif pour définir les CA de confiance, interdire le trafic clair, etc. [7] |

*Depuis Android 10 (API 29) TLS 1.3 est activé par défaut ; en-dessous, l’ajout du provider Conscrypt permet l’activation.*

---

### 2.3 · Gestion des certificats & confiance

* **Certificat auto-signé** : pratique pour tests, nécessite d’embarquer la racine dans le client (Android **assets/** → KeyStore).
* **TrustStore embarqué** : création d’un `KeyStore` programmatique + `TrustManager` dédié ; approche décrite dans plusieurs guides [8].
* **Authentification mutuelle (mTLS)** : le serveur demande le certificat du client (`SSL_CTX_set_verify`) ; exemple de code disponible [9]. Non implémentée dans la POC mais prévue.

---

## 3 · Choix d’implémentation

| Aspect | Décision | Motivation |
|--------|----------|------------|
| **Lib TLS serveur** | OpenSSL 3.2.0 | Présente dans dépôts Debian/Raspbian ; support TLS 1.3 ; communauté active |
| **Modèle multi-thread** | `pthread` + un thread par client | Simplicité ; performances suffisantes (< 20 conn. simult.) |
| **Certificats** | Auto-signé RSA-2048, SHA-256, val. 365 jours | 0 € ; renouvellement facile ; distribution contrôlée |
| **Protocole applicatif** | Texte `CMD\|PARAM\0` | Débogage aisé ; extensible vers Protobuf |
| **Client Android** | `SSLSocketFactory` custom + KeyStore embarqué | Compatible API 21+ ; pas besoin de privilèges root |

---

## 4 · Description des projets de mise en œuvre

### 4.1 · Sous-projet : Serveur C – Raspberry Pi

* **Rôle :** accepter des connexions TCP :5001, négocier TLS, interpréter commandes.
* **Modules principaux** :
  * `connection.*` : initialisation OpenSSL, création `SSL_CTX`, accept / lecture / écriture.
  * `communication.*` : boucle de réception par client (+ mutex journal).
  * `protocol.*` : sérialisation / désérialisation `CMD|PARAM`.
  * `server.c` : point d’entrée, gestion du pool de threads.
* **Tests** : client C inclus (`client/client.c`) ; exécutable via `make run-client`.

### 4.2 · Sous-projet : Client Android

* **Rôle :** créer une socket TLS vers le serveur, permettre d’envoyer/recevoir des commandes.
* **Classes clés** :
  * `SSLUtil.java` : chargement `server.crt` (assets) → `KeyStore` → `TrustManager`.
  * `Connection.java` : lecture/écriture I/O avec timeout.
  * `Communication.java` : thread de réception + callback UI.
  * `Protocol.java` : encodage/décodage `CMD|PARAM`.
  * `MainActivity.java` : UI (IP, logs, boutons *Connect/Send/Disconnect*).

---

## 5 · Perspectives

* **Protobuf** : remplacer le protocole texte pour sérialisation binaire efficace.
* **Authentification mutuelle** : ajouter un `KeyManager` côté Android et une autorité côté serveur.

---

## 6 · Références

1. *OpenSSL multithread safety* – openssl-threads(7). <https://docs.openssl.org/3.0/man7/openssl-threads/>  
2. *tls-server-block.c* – Exemple serveur OpenSSL. GitHub/openssl (2025-01-15). <https://github.com/openssl/openssl/blob/master/demos/guide/tls-server-block.c>  
3. wolfSSL – *Comparing wolfSSL vs OpenSSL* (blog, 2023-12). <https://www.wolfssl.com/comparing-wolfssl-vs-openssl/>  
4. ARM – *MbedTLS vs OpenSSL* (forum, 2019). <https://forums.mbed.com/t/comparing-mbedtls-to-openssl/6227>  
5. Android API – `SSLSocketFactory` doc. <https://developer.android.com/reference/javax/net/ssl/SSLSocketFactory>  
6. StackOverflow – *Enable TLS 1.3 with Conscrypt* (2019-04). <https://stackoverflow.com/questions/55539513/how-to-enable-tlsv1-3-for-okhttp-3-12-x-on-android-8-9>  
7. Android Dev – *Network Security Config* guide (2025-05). <https://developer.android.com/privacy-and-security/security-config>  
8. StackOverflow – *SSLSocket & self-signed certificates* (2014). <https://stackoverflow.com/questions/24520833/android-sslsockets-using-self-signed-certificates>  
9. StackOverflow – *Mutual TLS with OpenSSL C* (2018). <https://stackoverflow.com/questions/53952695/how-to-do-mutual-tls-authentication-in-c-code-using-openssl>

---

> Ce document est diffusé sous licence MIT, à l’instar des sous-projets sources.