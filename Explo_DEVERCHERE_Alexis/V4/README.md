# Secure TLS Chat - Android Client

![Version](https://img.shields.io/badge/version-4.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

A secure chat application for Android featuring TLS 1.2/1.3 encryption with certificate pinning.

## Features

- **Secure Communication**: TLS 1.2/1.3 encrypted chat
- **Certificate Pinning**: Enhanced security against MITM attacks
- **Modern Cipher Suites**: AES-256-GCM, ECDHE key exchange
- **User-Friendly Interface**:
  - Real-time message display
  - Connection status indicators
  - Auto-scrolling chat
- **Thread-Safe Implementation**: Background network operations

## Technical Specifications

- Minimum Android API: 21 (Lollipop)
- TLS Protocols: TLS 1.2, TLS 1.3
- Supported Cipher Suites:
  - TLS_AES_128_GCM_SHA256
  - TLS_AES_256_GCM_SHA384
  - TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
  - TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384

## Architecture

```mermaid
graph TD
    A[MainActivity] -->|UI Events| B[SecureChatClient]
    B -->|TLS Socket| C[TLSSocketFactory]
    C -->|Certificate Pinning| D[Server Certificate]
    B -->|Callbacks| A

Installation

    Clone the repository:
    bash

    git clone https://github.com/your-repo/secure-tls-chat.git

    Import into Android Studio

    Add your server certificate to res/raw/server_cert.crt

    Build and run

Usage

    Enter server IP and port

    Click "Connect"

    Send messages in the chat interface

Available commands:

    /nick <name> - Change your nickname

    /list - Show connected users

    /quit - Disconnect from server

Security Notes

    The debug socket factory (createDebugSocketFactory) should NEVER be used in production

    Always use proper certificate pinning in release builds

    Keep server certificates secure and rotated periodically

Dependencies

    AndroidX AppCompat

    OpenSSL (via Android's security provider)

License

MIT License

Copyright (c) 2023 Alexis DEVERCHERE

Permission is hereby granted... [include full license text]


The documentation provides:
1. Comprehensive file-level comments with version and author information
2. Detailed function documentation
3. Interface documentation
4. A complete README with:
   - Project overview
   - Features list
   - Technical specifications
   - Architecture diagram
   - Installation instructions
   - Usage guide
   - Security considerations
   - License information

All documentation follows consistent formatting and provides clear, actionable information for developers and users.