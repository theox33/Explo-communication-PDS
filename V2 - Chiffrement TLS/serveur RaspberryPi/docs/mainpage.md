# V2 - Chiffrement TLS - serveur RaspberryPi

## Présentation du projet

Ce sous-projet fait partie du projet global "V2 - Chiffrement TLS", dont l'objectif est de démontrer la possibilité d'établir une connexion sécurisée SSL/TLS sur une connexion TCP entre un serveur écrit en C et un client Android.

Le présent sous-projet, intitulé ** "V2 - Chiffrement TLS - serveur RaspberryPi" **, correspond à l'implémentation de la partie serveur en C.  
Il met en œuvre un serveur TCP sécurisé par TLS/SSL ([OpenSSL](https://www.openssl.org/)), capable de gérer plusieurs clients en parallèle grâce à des threads, et propose une interface de communication basée sur des protocoles personnalisés.


## Test avec un client C

Pour tester et valider le fonctionnement SSL/TLS de manière approfondie, un dossier ou fichier `client` est inclus dans ce dépôt.  
Ce client C permet de démontrer la capacité à échanger des messages sécurisés entre un serveur en C et un client en C, en utilisant la même infrastructure TLS.


## Compatibilité avec Android

Bien que ce sous-projet se concentre sur la partie serveur en C, il est important de noter que le serveur ainsi développé est également compatible avec un client Android (développé dans un autre sous-projet).  
Cela permet de valider l'interopérabilité entre différentes plateformes autour du protocole TLS.


## Structure du code

Le code est organisé autour de plusieurs modules :

| Module / Fichier      | Rôle                                                                                           |
|-----------------------|------------------------------------------------------------------------------------------------|
| `connection.h`        | Gestion des connexions réseau sécurisées via OpenSSL.                                          |
| `protocol.h`          | Encodage/décodage des messages échangés.                                                       |
| `communication.h`     | Gestion de la logique de communication et du threading.                                        |
| `server.c`            | Point d'entrée du serveur, gestion des clients et du cycle de vie des connexions.              |
| `client.c`            | Client C de test pour valider les échanges TLS.                                                |

## Architecture répertoire simplifiée

```text
.
├── client
│   └── src
│       └── client.c
├── common
│   ├── communication.c
│   ├── communication.h
│   ├── connection.c
│   ├── connection.h
│   ├── protocol.c
│   └── protocol.h
└── server
    ├── src
    │   ├── parser.c
    │   ├── parser.h
    │   └── server.c
    └── ssl
        ├── server.crt
        └── server.key
```

## Utilisation

### Étapes d'utilisation serveur/client C

1. Lancer le serveur sur le Raspberry Pi ou toute machine compatible.
2. Aller à la racine de ce sous-projet : `/V2 - Chiffrement TLS/serveur RaspberryPi/`
3. Utiliser le Makefile générique pour compiler le serveur et le client C :

    ```sh
    make all
    ```

4. Ouvrir deux terminaux et entrer :

    ```sh
    make run-server
    ```

    puis

    ```sh
    make run-client
    ```

### Étapes d'utilisation serveur C / client Android

- Il est également possible de connecter un client Android (voir le sous-projet correspondant) pour valider la compatibilité inter-plateformes.
- Effectuer les étapes précédentes à la différence que :

    1. Utiliser le Makefile générique pour compiler le serveur :

        ```sh
        make server
        ```

    2. Ouvrir un terminal et entrer :

        ```sh
        make run-server
        ```

---

## Extensibilité

* **Migration Protobuf** : remplacer `Protocol.c` par la sérialisation Protobuf. 

## Licence

Ce projet est distribué sous licence [MIT](https://opensource.org/licenses/MIT).

---
