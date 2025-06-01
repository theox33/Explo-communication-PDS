# Communication Client Java Android - Serveur C

## Vue d'ensemble du projet

Ce projet implémente une communication réseau entre une application Android (client Java) and un serveur en langage C, utilisant le protocole TCP/IP. L'architecture permet l'échange de messages textuels bidirectionnels en temps réel sur un réseau local.

## Architecture du système

### Composants principaux

**Serveur C (server.c)**
- Serveur TCP multi-connexions
- Écoute sur le port 8080
- Accepte les connexions entrantes
- Traite les messages et renvoie des réponses

**Client Android**
- Application mobile avec interface utilisateur
- Connexion TCP vers le serveur
- Envoi/réception de messages asynchrone
- Gestion des erreurs réseau

## Fonctionnalités

### Côté serveur C
- **Création de socket TCP** : Initialisation d'un socket serveur
- **Liaison et écoute** : Binding sur toutes les interfaces (INADDR_ANY)
- **Acceptation de connexions** : Gestion des connexions clients multiples
- **Communication bidirectionnelle** : Réception et envoi de messages
- **Logging** : Affichage des connexions et messages reçus
- **Nettoyage des ressources** : Fermeture propre des connexions

### Côté client Android
- **Interface utilisateur intuitive** : Champ de saisie et bouton d'envoi
- **Communication asynchrone** : Utilisation d'AsyncTask pour éviter le blocage de l'UI
- **Gestion d'erreurs** : Affichage des erreurs de connexion à l'utilisateur
- **Feedback visuel** : Toast notifications et affichage des réponses
- **Gestion des permissions** : Configuration automatique des permissions réseau

## Structure des fichiers

```
Projet/
├── Serveur C/
│   ├── server.c           # Code source du serveur
│   ├── Makefile          # Script de compilation
│   └── README_SERVER.md  # Instructions serveur
│
└── Client Android/
    ├── app/src/main/java/com/example/tcpclient/
    │   ├── MainActivity.java     # Activité principale
    │   └── TCPClient.java       # Classe de communication
    ├── app/src/main/res/layout/
    │   └── activity_main.xml    # Interface utilisateur
    └── app/src/main/
        └── AndroidManifest.xml  # Permissions et configuration
```

## Protocole de communication

### Format des messages
- **Encoding** : UTF-8
- **Délimiteur** : Caractère de nouvelle ligne (\n)
- **Taille maximale** : 1024 octets par message

### Flux de communication
1. **Établissement de connexion** : Le client se connecte au serveur via TCP
2. **Envoi de message** : Le client envoie un message texte
3. **Traitement** : Le serveur reçoit et traite le message
4. **Réponse** : Le serveur renvoie une confirmation
5. **Fermeture** : La connexion se ferme automatiquement après chaque échange

## Configuration réseau

### Paramètres par défaut
- **Port** : 8080
- **Protocole** : TCP
- **Interface** : Toutes les interfaces (0.0.0.0)
- **Timeout** : Aucun (connexion persistante côté serveur)

### Configuration requise
- Serveur et client sur le même réseau local
- Port 8080 non bloqué par le firewall
- Permissions réseau accordées sur Android

## Installation et déploiement

### Prérequis
- **Serveur** : Compilateur GCC, système Unix/Linux
- **Client** : Android Studio, SDK Android (API 21+)
- **Réseau** : Connexion WiFi commune

### Étapes de déploiement

**1. Compilation du serveur**
```bash
gcc -o server server.c
# ou utiliser le Makefile
make
```

**2. Lancement du serveur**
```bash
./server
```

**3. Configuration du client**
- Ouvrir le projet dans Android Studio
- Modifier l'IP dans `TCPClient.java` ligne 12
- Compiler et installer sur l'appareil Android

**4. Test de connexion**
- Lancer le serveur C
- Ouvrir l'application Android
- Saisir un message et appuyer sur "Envoyer"

## Sécurité et limitations

### Considérations de sécurité
- **Trafic non chiffré** : Communication en texte clair
- **Authentification** : Aucune authentification implémentée
- **Validation d'entrée** : Validation basique côté client uniquement
- **Réseau local uniquement** : Non sécurisé pour Internet

### Limitations actuelles
- **Connexions simultanées** : Une seule connexion traitée à la fois
- **Persistance** : Pas de stockage des messages
- **Reconnexion** : Pas de mécanisme de reconnexion automatique
- **Taille des messages** : Limitée à 1024 caractères

## Dépannage

### Erreurs courantes

**"Connection refused"**
- Vérifier que le serveur est en cours d'exécution
- Contrôler l'adresse IP et le port
- Vérifier la connectivité réseau

**"Operation not permitted"**
- Ajouter les permissions Internet dans AndroidManifest.xml
- Vérifier les paramètres de sécurité Android

**"Network unreachable"**
- Confirmer que les appareils sont sur le même réseau
- Tester la connectivité avec ping

### Debug et logs
- **Serveur** : Messages dans la console système
- **Client** : Logs Android accessibles via `adb logcat`
- **Réseau** : Utiliser `netstat` pour vérifier les ports ouverts

## Extensions possibles

### Améliorations suggérées
- **Chiffrement SSL/TLS** pour sécuriser les communications
- **Base de données** pour persister les messages
- **Interface web** pour administration du serveur
- **Reconnexion automatique** côté client
- **Gestion multi-threading** côté serveur
- **Compression** des messages volumineux
- **Heartbeat** pour détecter les déconnexions

### Évolutions avancées
- **Architecture microservices** avec load balancer
- **Support WebSocket** pour communication temps réel
- **API REST** en complément du TCP
- **Monitoring** et métriques de performance
- **Tests automatisés** unitaires et d'intégration

## Support et maintenance

### Contact
- Développement initial : [Votre nom/équipe]
- Date de création : [Date actuelle]
- Version : 1.0

### Maintenance
- **Tests réguliers** : Vérification de compatibilité réseau
- **Mises à jour sécurité** : Surveillance des vulnérabilités
- **Performance** : Monitoring des temps de réponse
- **Documentation** : Mise à jour selon les évolutions