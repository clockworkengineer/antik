# Antik C++ Library: Architecture & Technical Design Guide

This document details the architectural design principles, interface abstractions, component organization, and thread-safety models governing the **Antik C++ Library**.

---

## 1. Architectural Philosophy & SOLID Principles

The Antik library is structured strictly according to the **SOLID** principles of Object-Oriented Design:

```
+---------------------------------------------------------------------------------------------------+
|                                 SOLID DESIGN ENFORCEMENT                                          |
+-----+----------------------------------+--------------------------------------------------------+
| Key | Principle                        | Enforced Implementation Design                         |
+-----+----------------------------------+--------------------------------------------------------+
| S   | Single Responsibility (SRP)      | MailMessageBuilder isolates MIME formatting from       |
|     |                                  | CSMTP network transmission logic.                      |
+-----+----------------------------------+--------------------------------------------------------+
| O   | Open/Closed (OCP)                | IRemoteFileSystem allows adding WebDAV or S3 backends  |
|     |                                  | without modifying transfer utility code.               |
+-----+----------------------------------+--------------------------------------------------------+
| L   | Liskov Substitution (LSP)        | IFileEventNotifier provides substitutable default      |
|     |                                  | implementations across watcher types.                  |
+-----+----------------------------------+--------------------------------------------------------+
| I   | Interface Segregation (ISP)      | IApprise segregated into IEventListener, IWatchControl,|
|     |                                  | and IWatchTargetManager role interfaces.               |
+-----+----------------------------------+--------------------------------------------------------+
| D   | Dependency Inversion (DIP)       | High-level utilities depend on ISSHSession &           |
|     |                                  | IRemoteFileSystem abstractions instead of concrete C wrappers|
+-----+----------------------------------+--------------------------------------------------------+
```

---

## 2. Component Layer Architecture

```
+---------------------------------------------------------------------------------------------------+
|                                 COMPONENT LAYER ARCHITECTURE                                      |
+---------------------------------------------------------------------------------------------------+
|                                        APPLICATION LAYER                                          |
|                     (FTPBackup, FTPSync, SMTPSendMail, FileWatcher, Custom Apps)                  |
+---------------------------------------------------------------------------------------------------+
|                                       ABSTRACTION LAYER                                           |
|       (IRemoteFileSystem, ISSHSession, IAppriseComposite, IArchive, ConnectionPool<T>)            |
+---------------------------------------------------------------------------------------------------+
|                                   PROTOCOL & PROVIDER LAYER                                       |
|             (CFTP, CSFTP, CSCP, CWebDAV, CSMTP, CIMAP, MailMessageBuilder, CZIP)                  |
+---------------------------------------------------------------------------------------------------+
|                                      LOW-LEVEL C DRIVER LAYER                                     |
|                              (CCurl, CSocket, libssh, libcurl, OpenSSL)                           |
+---------------------------------------------------------------------------------------------------+
```

---

## 3. Core Interface Abstractions

### 3.1 `IRemoteFileSystem`
Defined in [`include/IRemoteFileSystem.hpp`](file:///home/robt/projects/antik/include/IRemoteFileSystem.hpp).
Provides protocol-agnostic remote file system operations:
- `changeDir`, `getCurrentWorkingDirectory`, `createDirectory`, `isDirectory`, `isFile`
- `uploadFile`, `downloadFile`, `uploadFileAsync`, `downloadFileAsync`
- `listDirectory`

### 3.2 `ISSHSession`
Defined in [`include/ISSHSession.hpp`](file:///home/robt/projects/antik/include/ISSHSession.hpp).
Abstracts low-level SSH connection lifecycle:
- `setServer`, `setUserAndPassword`, `connect`, `disconnect`, `isConnected`, `getError`

### 3.3 `IArchive`
Defined in [`include/IArchive.hpp`](file:///home/robt/projects/antik/include/IArchive.hpp).
Abstracts compression archive operations:
- `open`, `extractTo`, `addFile`, `listContents`

---

## 4. Threading & Synchronization Model

- **`ThreadQueue<T>`**: Thread-safe producer-consumer queue guarded by `std::mutex` and `std::condition_variable`.
- **`ConnectionPool<T>`**: Thread-safe resource pool managing reusable client instances across worker threads.
- **`CApprise`**: Dedicated background thread executing the `watch()` loop to generate `IApprise::Event` notifications asynchronously.
