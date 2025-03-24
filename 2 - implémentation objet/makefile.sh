#/bin/bash

# Compile with debugging info
gcc -g -c protocol.c -o protocol.o
gcc -g -c connection.c -o connection.o
gcc -g -c communication.c -o communication.o

# Compile server and client
gcc -g -o server server.c protocol.o connection.o communication.o -lpthread
gcc -g -o client client.c protocol.o connection.o communication.o -lpthread