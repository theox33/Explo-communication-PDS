CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -lssl -lcrypto -pthread

# Détection de l'OS pour les chemins OpenSSL
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS avec Homebrew
    CFLAGS += -I/usr/local/opt/openssl/include
    LDFLAGS += -L/usr/local/opt/openssl/lib
endif

TARGET = tls_server
CLIENT_TARGET = tls_client

all: $(TARGET) $(CLIENT_TARGET)

$(TARGET): tls_server.c
	$(CC) $(CFLAGS) -o $(TARGET) tls_server.c $(LDFLAGS)

$(CLIENT_TARGET): tls_client.c
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) tls_client.c $(LDFLAGS)

clean:
	rm -f $(TARGET) $(CLIENT_TARGET)

run-server: $(TARGET)
	./$(TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

.PHONY: all clean run-server run-client