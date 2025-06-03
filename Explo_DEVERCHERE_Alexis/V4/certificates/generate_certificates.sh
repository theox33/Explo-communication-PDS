#!/bin/bash

# Script de génération des certificats auto-signés pour le serveur TLS

echo "=== Génération des certificats TLS auto-signés ==="

# Configuration
CERT_DIR="./certificates"
SERVER_KEY="server.key"
SERVER_CERT="server.crt"
SERVER_PEM="server.pem"
CLIENT_TRUSTSTORE="client_truststore.bks"
VALIDITY_DAYS=365
SERVER_IP="192.168.0.31"  # Remplacez par l'IP de votre serveur

# Créer le répertoire des certificats
mkdir -p $CERT_DIR
cd $CERT_DIR

# 1. Générer la clé privée du serveur
echo "1. Génération de la clé privée du serveur..."
openssl genrsa -out $SERVER_KEY 2048

# 2. Générer le certificat auto-signé
echo "2. Génération du certificat auto-signé..."
openssl req -new -x509 -key $SERVER_KEY -out $SERVER_CERT -days $VALIDITY_DAYS \
    -subj "/C=FR/ST=France/L=Paris/O=MyOrg/OU=Dev/CN=$SERVER_IP" \
    -addext "subjectAltName = IP:$SERVER_IP"

# 3. Créer le fichier PEM combiné pour le serveur C
echo "3. Création du fichier PEM..."
cat $SERVER_KEY $SERVER_CERT > $SERVER_PEM

# 4. Exporter le certificat pour Android (format DER)
echo "4. Export du certificat pour Android..."
openssl x509 -in $SERVER_CERT -outform DER -out server_cert.der

# 5. Créer un truststore BKS pour Android (optionnel)
echo "5. Création du truststore BKS pour Android..."
# Note: Nécessite keytool avec le provider BouncyCastle
if command -v keytool &> /dev/null; then
    keytool -importcert -v -trustcacerts \
        -alias server \
        -file $SERVER_CERT \
        -keystore $CLIENT_TRUSTSTORE \
        -storetype BKS \
        -provider org.bouncycastle.jce.provider.BouncyCastleProvider \
        -storepass android \
        -noprompt 2>/dev/null || echo "Note: BKS creation skipped (BouncyCastle provider needed)"
fi

# 6. Afficher les informations du certificat
echo -e "\n=== Informations du certificat ==="
openssl x509 -in $SERVER_CERT -text -noout | grep -E "(Subject:|Not Before|Not After|Subject Alternative Name)" -A 1

echo -e "\n=== Fichiers générés ==="
ls -la

echo -e "\n=== Instructions ==="
echo "1. Copiez 'server.pem' sur votre serveur Linux"
echo "2. Copiez 'server_cert.der' dans app/src/main/res/raw/ de votre projet Android"
echo "3. Renommez 'server_cert.der' en 'server_cert.crt' dans Android Studio"
echo "4. Mettez à jour YOUR_SERVER_IP dans network_security_config.xml avec: $SERVER_IP"