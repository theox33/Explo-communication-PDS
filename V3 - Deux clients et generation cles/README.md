# 🔐 Système de Communication Sécurisée C/Android

Un système de messagerie client-serveur cross-platform avec chiffrement SSL/TLS, implémenté en C pour le serveur et Java pour le client Android.

## 📋 Table des Matières

- [Vue d'ensemble](#vue-densemble)
- [Architecture](#architecture)
- [Côté Serveur (C)](#côté-serveur-c)
- [Côté Client Android (Java)](#côté-client-android-java)
- [Installation](#installation)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Sécurité](#sécurité)
- [API et Protocole](#api-et-protocole)
- [Monitoring](#monitoring)
- [Dépannage](#dépannage)
- [Contribution](#contribution)

## 🌟 Vue d'ensemble

Ce projet implémente un système de communication sécurisée permettant à des clients Android de se connecter à un serveur C via SSL/TLS. Le système supporte la messagerie multi-clients, la gestion dynamique des certificats et un monitoring avancé.

### ✨ Fonctionnalités principales

- 🔒 **Chiffrement SSL/TLS** avec gestion automatique des certificats
- 📱 **Client Android natif** avec interface utilisateur moderne
- 🌐 **Multi-clients** avec diffusion de messages
- 📊 **Monitoring temps réel** avec métriques de performance
- 🔄 **Reconnexion automatique** et gestion d'erreurs
- 🛡️ **Sécurité renforcée** avec validation des certificats
- 📝 **Logging détaillé** pour debug et audit

## 🏗️ Architecture

```
┌─────────────────┐     SSL/TLS     ┌─────────────────┐
│  Client Android │ ◄─────────────► │   Serveur C     │
│   (Java/Kotlin) │                 │    (OpenSSL)    │
└─────────────────┘                 └─────────────────┘
         │                                   │
         ▼                                   ▼
┌─────────────────┐                 ┌─────────────────┐
│ Interface UI    │                 │ Multi-threading │
│ Message Handler │                 │ Client Manager  │
│ SSL Manager     │                 │ SSL Context     │
└─────────────────┘                 └─────────────────┘
```

## 💻 Côté Serveur (C)

### 📁 Structure des fichiers

```
server/
├── main.c              # Point d'entrée principal
├── server.c/.h         # Logique serveur principal
├── client_manager.c/.h # Gestion des clients connectés
├── ssl_utils.c/.h      # Utilitaires SSL/TLS
├── communication.c/.h  # Protocole de communication
└── logger.c/.h         # Système de logging
```

### 🔧 Composants principaux

#### **Server (`server.c`)**
- Serveur SSL multi-threadé
- Gestion des connexions simultanées
- Pool de threads pour performance
- Monitoring de santé système

```c
typedef struct {
    int server_fd;
    SSL_CTX *ssl_ctx;
    ThreadPool *thread_pool;
    ClientManager *client_manager;
    ServerStats stats;
} Server;
```

#### **ClientManager (`client_manager.c`)**
- Registre des clients connectés
- Diffusion de messages (broadcast)
- Gestion du cycle de vie des connexions
- Statistiques par client

```c
typedef struct {
    ClientInfo *clients[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t mutex;
    BroadcastStats stats;
} ClientManager;
```

#### **SSL Utils (`ssl_utils.c`)**
- Configuration OpenSSL
- Gestion des certificats
- Contexte de sécurité
- Validation SSL/TLS

#### **Communication (`communication.c`)**
- Protocole de messages `CMD|PARAM`
- Encodage/décodage binaire
- Gestion des timeouts
- Validation des données

#### **Logger (`logger.c`)**
- Logging multi-niveaux (DEBUG, INFO, WARN, ERROR)
- Rotation automatique des logs
- Thread-safe logging
- Export de métriques

### ⚡ Fonctionnalités serveur

- **Multi-threading** : Un thread par client + pool de workers
- **SSL/TLS** : Chiffrement bout-en-bout avec OpenSSL
- **Broadcasting** : Diffusion de messages à tous les clients
- **Heartbeat** : Détection de déconnexions
- **Monitoring** : Métriques temps réel et alertes
- **Graceful shutdown** : Arrêt propre avec nettoyage

## 📱 Côté Client Android (Java)

### 📁 Structure des fichiers

```
android/src/main/java/com/example/server_client/
├── MainActivity.java    # Interface utilisateur principale
├── Client.java         # Client de communication principal
├── Connection.java     # Gestion connexion SSL/TLS
├── Communication.java  # Thread de communication
├── Protocol.java       # Protocole de messages
└── SSLUtil.java        # Utilitaires SSL Android
```

### 🔧 Composants principaux

#### **Client (`Client.java`)**
- Interface principale pour l'application
- Gestion des callbacks de messages
- Coordination des composants
- API simple pour l'UI

```java
public class Client {
    public interface MessageListener {
        void onMessageReceived(String message, String senderId);
    }
    
    public boolean connectToServer();
    public boolean sendMessage(String message);
    public void setMessageListener(MessageListener listener);
}
```

#### **Connection (`Connection.java`)**
- Connexions SSL/TLS sécurisées
- Gestion dynamique des certificats
- Reconnexion automatique
- Validation des certificats serveur

#### **Communication (`Communication.java`)**
- Thread de réception des messages
- Gestion des timeouts réseau
- Monitoring de santé de connexion
- Buffer circulaire pour performance

#### **MainActivity (`MainActivity.java`)**
- Interface utilisateur intuitive
- Gestion des threads UI/réseau
- Affichage des messages en temps réel
- Contrôles de connexion/déconnexion

#### **SSLUtil (`SSLUtil.java`)**
- Trust store Android personnalisé
- Gestion des certificats auto-signés
- Sauvegarde de certificats
- Configuration SSL adaptée mobile

### 🎨 Interface utilisateur

- **Material Design** moderne et responsive
- **Messagerie en temps réel** avec auto-scroll
- **Indicateurs de statut** (connecté/déconnecté)
- **Notifications toast** pour feedback utilisateur
- **Gestion d'erreurs** avec messages explicites

## 🚀 Installation

### Prérequis

**Serveur C :**
- GCC 7.0+ ou Clang
- OpenSSL 1.1.1+
- CMake 3.10+
- Pthreads

**Client Android :**
- Android Studio 4.0+
- Android SDK API 21+ (Android 5.0)
- Gradle 6.0+

### Compilation

#### Serveur C

```bash
# Cloner le projet
git clone https://github.com/votre-repo/secure-messaging
cd secure-messaging/server

# Créer le dossier de build
mkdir build && cd build

# Configurer avec CMake
cmake ..

# Compiler
make -j$(nproc)

# Exécuter les tests
make test
```

#### Client Android

```bash
# Ouvrir dans Android Studio
cd ../android
./gradlew build

# Installer sur appareil/émulateur
./gradlew installDebug
```

## ⚙️ Configuration

### Configuration serveur

**Fichier `config/server.conf` :**

```ini
[network]
port=5001
max_clients=100
bind_address=0.0.0.0

[ssl]
cert_file=certs/server.crt
key_file=certs/server.key
ca_file=certs/ca.crt
verify_client=false

[logging]
level=INFO
file=logs/server.log
max_size=100MB
rotate_count=5

[performance]
thread_pool_size=10
buffer_size=8192
timeout_seconds=30
```

### Configuration Android

**Modifier `Client.java` :**

```java
private static final String SERVER_IP = "192.168.1.100";
private static final int PORT = 5001;
```

**Certificats SSL :**
- Placer `server.crt` dans `assets/`
- Le client récupère automatiquement les certificats mis à jour

## 📖 Utilisation

### Démarrer le serveur

```bash
# Démarrage normal
./server

# Mode debug
./server --debug --log-level DEBUG

# Configuration personnalisée
./server --config /path/to/config.conf
```

### Utiliser l'application Android

1. **Lancer l'app** sur votre appareil Android
2. **Appuyer sur "Connect"** pour se connecter au serveur
3. **Saisir un message** et appuyer sur "Send"
4. **Voir les messages** des autres clients en temps réel
5. **Appuyer sur "Disconnect"** pour se déconnecter proprement

### Commandes serveur

```bash
# Statut en temps réel
curl http://localhost:8080/stats

# Liste des clients connectés
curl http://localhost:8080/clients

# Arrêt gracieux
kill -TERM $(pidof server)
```

## 🔒 Sécurité

### Chiffrement

- **SSL/TLS 1.2+** minimum requis
- **Chiffrements forts** uniquement (AES-256, ChaCha20)
- **Perfect Forward Secrecy** avec courbes elliptiques
- **Validation de certificats** stricte

### Certificats

```bash
# Générer certificats de développement
./scripts/generate-certs.sh

# Certificats auto-signés pour test
openssl req -x509 -newkey rsa:4096 -keyout server.key \
    -out server.crt -days 365 -nodes
```

### Sécurité Android

- **Certificate Pinning** dynamique
- **Trust Store** personnalisé
- **Validation hostname** activée
- **Protection contre MitM**

## 📡 API et Protocole

### Protocole de messages

**Format :** `COMMANDE|PARAMETRE\0`

```
CMD_X|Hello World\0     # Message client vers serveur
CMD_Y|Client 2: Hi!\0   # Message serveur vers clients
```

### API REST (monitoring)

```bash
GET /stats              # Statistiques serveur
GET /clients            # Liste des clients
GET /health             # Santé système
POST /broadcast         # Envoyer message à tous
DELETE /client/:id      # Déconnecter client
```

### Callbacks Android

```java
client.setMessageListener(new Client.MessageListener() {
    @Override
    public void onMessageReceived(String message, String senderId) {
        // Traiter message reçu
        runOnUiThread(() -> updateUI(message, senderId));
    }
});
```

## 📊 Monitoring

### Métriques serveur

- **Connexions** : actives, totales, échouées
- **Messages** : envoyés, reçus, erreurs
- **Performance** : latence, débit, CPU, mémoire
- **SSL** : handshakes, erreurs de certificat

### Logs structurés

```json
{
  "timestamp": "2025-01-15T10:30:00Z",
  "level": "INFO",
  "component": "ClientManager",
  "event": "client_connected",
  "client_id": "android_001",
  "ip": "192.168.1.50"
}
```

### Dashboard (optionnel)

Intégration possible avec :
- **Grafana** + InfluxDB pour métriques
- **ELK Stack** pour logs
- **Prometheus** pour alerting

## 🔧 Dépannage

### Problèmes courants

**Serveur ne démarre pas :**
```bash
# Vérifier port disponible
netstat -tlnp | grep 5001

# Vérifier certificats SSL
openssl x509 -in server.crt -text -noout
```

**Client Android ne se connecte pas :**
- Vérifier IP serveur dans `Client.java`
- Contrôler pare-feu/NAT
- Valider certificat SSL

**Messages perdus :**
- Vérifier logs serveur pour erreurs
- Contrôler stabilité réseau
- Ajuster timeouts si nécessaire

### Debug mode

**Serveur :**
```bash
# Logs détaillés
export SSL_DEBUG=1
./server --debug

# Analyser trafic SSL
tcpdump -i any -w capture.pcap port 5001
```

**Android :**
```java
// Activer logs SSL dans SSLUtil.java
System.setProperty("javax.net.debug", "ssl:handshake");
```

## 🤝 Contribution

### Standards de développement

- **Documentation** : Doxygen pour C, JavaDoc pour Android
- **Tests** : Unit tests obligatoires pour nouvelles fonctionnalités
- **Code style** : GNU pour C, Google Java Style pour Android
- **Git** : Conventional Commits

### Roadmap

- [ ] Support IPv6
- [ ] Compression des messages (zstd)
- [ ] Authentification JWT
- [ ] Client desktop (Qt/Electron)
- [ ] Clustering serveur
- [ ] Push notifications Android

## 📄 Licence

MIT License - voir [LICENSE.md](LICENSE.md)

## 👨‍💻 Auteur

**Alexis DEVERCHERE** - Version 3.0 (2025)

---

## 📞 Support

- **Issues** : [GitHub Issues](https://github.com/votre-repo/issues)
- **Documentation** : [Wiki du projet](https://github.com/votre-repo/wiki)
- **Contact** : alexis.deverchere@reseau.eseo.fr