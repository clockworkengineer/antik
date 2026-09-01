# Antik C++ Library #

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.txt)
[![Build & Test](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Donate-orange.svg)](https://buymeacoffee.com/roberttizz1)

**Antik** is an enterprise-grade, modern C++ library providing cross-platform networking, remote filesystem management (FTP/S, SFTP, SCP, WebDAV), email messaging (SMTP/S, IMAP/S with XOAUTH2 support), file monitoring (`inotify`/Apprise), archive handling (ZIP), and thread-safe utility abstractions built strictly according to **SOLID Object-Oriented Design Principles**.

---

## Key Features & Highlights

- **SOLID Architecture**: Designed around loose coupling, pure interfaces, role segregation, and dependency inversion.
- **Unified Remote File System (`IRemoteFileSystem`)**: Single interface to manage remote file systems across FTP, SFTP, SCP, and WebDAV storage providers.
- **Transfer Progress Callbacks & Speed Monitoring**: `TransferProgress` structure and `ProgressFn` callbacks for byte counts, rates (B/s), and progress percentages.
- **Interrupted Transfer Resume**: FTP `REST` and SFTP offset-based upload/download resumption.
- **Email & Messaging**: MIME formatting via `MailMessageBuilder`, SMTP/S mail delivery (`CSMTP`), and IMAP/S mailbox management (`CIMAP`) with SASL XOAUTH2 token authentication.
- **File System Event Monitoring**: Event-driven file watching using `CApprise` and `IFileEventNotifier` with regex/glob pattern filtering.
- **Archiving**: Unified `IArchive` interface and ZIP archive handling (`CZIP`).
- **Thread-Safe Utilities**: Non-blocking `ThreadQueue<T>` and `ConnectionPool<T>` connection management.

---

## Architecture & SOLID Principles

```
+---------------------------------------------------------------------------------------------------+
|                                      ANTIK DESIGN HIERARCHY                                       |
+--------------------------+------------------------------------------------------------------------+
| Interface                | Implementations / Extensions                                           |
+--------------------------+------------------------------------------------------------------------+
| IRemoteFileSystem        | CFTP, CSFTP, CSCP, CWebDAV                                             |
| ISSHSession              | CSSHSession                                                            |
| IAppriseComposite        | CApprise                                                               |
| IFileEventNotifier       | CFileEventNotifier                                                     |
| IArchive                 | CZIP                                                                   |
+--------------------------+------------------------------------------------------------------------+
```

---

## Core Components Overview

### Network & Remote File Systems (`IRemoteFileSystem`)

#### `CFTP`
FTP and FTPS (explicit TLS) remote storage client providing directory listing, navigation, creation, and offset-based file transfer resumption.

#### `CSFTP` & `CSCP`
SSH-2 SFTP and SCP remote storage providers built on `libssh` implementing `IRemoteFileSystem` and `ISSHSession`.

#### `CWebDAV`
WebDAV HTTP/S remote storage provider powering Nextcloud, OwnCloud, and WebDAV servers.

---

### Email & Messaging

#### `MailMessageBuilder`
Decoupled MIME builder isolating MIME header formatting, Date generation, Base64 attachment payload construction, and HTML/text dual body formatting.

#### `CSMTP` & `CIMAP`
CURL-backed SMTP client and TLS-secured IMAP client supporting standard password and SASL XOAUTH2 bearer token authentication.

---

### Archiving & Compression (`IArchive`)

#### `CZIP`
ZIP archive reader and creator implementing `IArchive` with entry extraction, compression, and file listing.

---

### Utilities & Connection Management

#### `ConnectionPool<T>`
Thread-safe connection pool for acquiring and returning reusable `IRemoteFileSystem` connections across worker threads.

#### `StringUtil` & `CPath`
Zero-allocation string manipulation helpers powered by `std::string_view` (`trim`, `startsWith`, `endsWith`, `joinServerPath`).

---

## Documentation Suite

Detailed technical guides and reference documentation are available in the [`docs/`](docs/) directory:

- 📘 [**Architecture Guide**](docs/ARCHITECTURE.md): Class hierarchy, SOLID design principles, and threading models.
- 📙 [**API Reference**](docs/API_REFERENCE.md): Detailed API manual for all public classes and headers.
- 📗 [**Code Examples**](docs/EXAMPLES.md): Copy-paste recipes for file transfers, email sending, directory watching, and connection pooling.

---

## Building and Testing

### Prerequisites
- C++17 compliant compiler (GCC 9+, Clang 10+)
- CMake 3.10.2+
- OpenSSL, libssh, libcurl, Boost C++ Libraries

### Build Instructions

```bash
# Configure build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build static library, unit tests, and examples
cmake --build build

# Run unit test suite
ctest --test-dir build --output-on-failure
```

---

## Support the Project

If you find Antik helpful in your projects, feel free to support ongoing development:

[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Donate-orange.svg?style=for-the-badge)](https://buymeacoffee.com/roberttizz1)

👉 [**Buy Me a Coffee**](https://buymeacoffee.com/roberttizz1)

---

## License

Antik is released under the [MIT License](LICENSE.txt).