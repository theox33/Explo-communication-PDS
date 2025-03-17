Les `sockets` sont les points d'extrémité virtuels de tout type de communication réseau entre deux hôtes sur un réseau.

Aujourd'hui, la norme officielle la plus récente est l'api `posix sockets`.

Le but de cette Issue et de ses sous-issues est **d'expliquer ce que sont les sockets et comment les manipuler de manière simple et générale**.

# Socket client
Un client est une application qui se connecte à un système distant pour obtenir ou récupérer des données.
Les étapes principales à suivre sont :

1. Créer un Socket
2. Se connecter à un serveur distant
3. Envoyer des données
4. Recevoir une réponse

> [!WARNING]
> Dans le cadre du projet *ProSE*, la connexion TCP doit s'effectuer entre des `client  Android` et un `serveur Linux en C`.
> La connexion client étant ainsi implémentée dans une application Android, **il ne sera pas question de détailler la création d'un client en C sur Linux** mais <ins>uniquement</ins> d'un `serveur`.

# Socket serveur

L'autre type d'application de socket est donc appelé `serveur` de socket.

> Un serveur est un système qui utilise des sockets pour recevoir des connexions entrantes et leur fournir des données.

Les serveurs sont à *l'opposé des clients*, c'est-à-dire qu'au lieu de se connecter aux autres, ils attendent les connexions entrantes.

Les serveurs de sockets fonctionnent donc de la manière suivante :

1. Ouvrir un socket
2. Se lier à une adresse (et à un port)
3. Écouter les connexions entrantes
4. Accepter les connexions
5. Lire/envoyer des données

La première étape est donc d'ouvrir *(créer)* un socket côté serveur.

## Créer un socket

```c
int socket_desc;
struct sockaddr_in server;
	
// Créer un socket
socket_desc = socket(AF_INET , SOCK_STREAM , 0);
if (socket_desc == -1) printf("Could not create socket");
```

La fonction `socket()` crée un socket et retourne un descripteur qui sera utilisé par d'autres fonctions. Le code ci-dessus créera un socket avec les propriétés suivantes :

* **Famille d'adresses** : `AF_INET` *(il s'agit de l'IPv4)*
* **Type** : `SOCK_STREAM` *(=connexion orientée vers le protocole TCP)*
* **Protocole** : `0` ou `IPPROTO_IP`, c'est le protocole IP

> [!TIP]
> **Remarque** : En dehors du type de socket `SOCK_STREAM`, il existe un autre type appelé `SOCK_DGRAM`, qui indique le protocole UDP. Ce type de socket est non connecté. Dans notre cas, nous nous en tiendrons à `SOCK_STREAM` ou aux sockets TCP. Cependant, il pourra être utilisé pour l'exploration du flux vidéo.

## Se lier *(`bind`)* à une adresse/port

La fonction `bind` peut être utilisée pour lier un socket à une combinaison spécifique "adresse et port". Elle nécessite une structure `sockaddr_in` similaire à celle utilisée par la fonction `connect`.

```c
int socket_desc;
struct sockaddr_in server;
	
// Créer un socket
socket_desc = socket(AF_INET , SOCK_STREAM , 0);
if (socket_desc == -1) printf("Could not create socket");

// Préparer la structure sockaddr_in
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;
server.sin_port = htons(8888);

// Lier le socket
if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) 
    puts("bind failed");
puts("bind done");
```

La structure `sockaddr_in` est utilisée pour stocker les informations relatives à l'adresse du socket.

`server.sin_family = AF_INET;`
Définit la famille d'adresses à AF_INET, ce qui signifie que nous utilisons le protocole IPv4.

`server.sin_addr.s_addr = INADDR_ANY;`
Associe le socket à toutes les interfaces réseau disponibles sur la machine.
Cela permet au serveur d'écouter les connexions entrantes depuis n'importe quelle adresse IP associée à l'hôte.

`server.sin_port = htons(8888);`
Définit le port sur lequel le serveur écoutera les connexions.
`htons(8888)` convertit le numéro de port en format "network byte order" (Big Endian), qui est la convention utilisée par les protocoles réseau.

La fonction bind est utilisée pour associer le socket à une adresse IP et un port spécifiques.

`if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) 
    puts("bind failed");` Tente de lier le socket à l'adresse et au port définis dans la structure server.
Si bind échoue (retourne une valeur négative), un message "bind failed" est affiché.

`puts("bind done");`
Si bind réussit, un message "bind done" est affiché pour indiquer que l'association a été effectuée avec succès.

> [!WARNING]
> Il faut faire attention à l'attribution du port dans cette étape. Il y a en effet des ports déjà utilisés par le système tels que 22 pour ssh, 443 pour https, 80 pour http... 8888 n'étant pas utilisé par autre chose à ce moment, il est bon candidat. Il faudra faire attention à ne pas réutiliser ce port pour autre chose à l'avenir.

Cela signifie évidemment qu'on ne peut pas avoir deux sockets liés au même port.


Maintenant que la liaison (bind) est effectuée, il faut faire en sorte que le socket écoute les connexions entrantes de la part des clients Android.

## Ecouter les connexions entrantes sur le socket
Après avoir lié un socket à un port de connexion *(ici, 8888)*, il faut écouter les connexions entrantes.

La fonction `listen` permet au socket de se mettre en mode écoute : 
```c
// Listen
listen(socket_desc , 3);
```

## Accepter une connexion
La fonction `accept`  extrait la première demande de connexion de la file d'attente des connexions en attente pour le socket d'écoute. `sockfd`, crée un nouveau socket connecté et retourne un nouveau descripteur de fichier faisant référence à ce socket. Le socket nouvellement créé n'est pas dans l'état d'écoute. Le socket original sockfd n'est pas affecté par cet appel.

```c
//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (new_socket<0) perror("accept failed");
	
	puts("Connection accepted");

	return 0;
```

Ici,

```c
c = sizeof(struct sockaddr_in);
```
initialise la variable c avec la taille de la structure sockaddr_in, qui stocke les informations sur le client (adresse IP et port).

```c
new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
```
Acceptation d'une connexion entrante :

- La fonction `accept()` extrait une connexion en attente de la file d'attente du socket d'écoute socket_desc.
- Elle crée un nouveau socket connecté (`new_socket`) qui permettra de communiquer avec le client.
- Le second paramètre `(struct sockaddr *)&client` permet de stocker l'adresse du client.
- Le troisième paramètre `(socklen_t*)&c` transmet la taille de la structure `sockaddr_in` (obligatoire pour certaines implémentations de `accept()`).

```c
if (new_socket<0) perror("accept failed");
```
Vérifie si `accept()` a échoué (`new_socket < 0`) et affiche un message d'erreur avec `perror()`.

-------------------

> @sources : 
> https://www.binarytides.com/socket-programming-c-linux-tutorial/
