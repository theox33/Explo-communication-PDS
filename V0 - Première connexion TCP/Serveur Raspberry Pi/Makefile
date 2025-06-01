# Makefile pour le serveur TCP en C

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = server
SOURCES = server.c

# Règle par défaut
all: $(TARGET)

# Compilation du serveur
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Nettoyage
clean:
	rm -f $(TARGET)

# Exécution du serveur
run: $(TARGET)
	./$(TARGET)

# Installation des dépendances (si nécessaire)
install:
	@echo "Aucune dépendance externe requise"

.PHONY: all clean run install