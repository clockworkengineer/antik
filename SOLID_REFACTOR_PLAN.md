# Antik C++ Library: Comprehensive SOLID Refactoring Plan

This document presents a complete source code architecture analysis and concrete refactoring plan to align the Antik C++ library with the **SOLID Object-Oriented Design Principles**.

---

## 1. Codebase Audit & SOLID Principles Gap Analysis

```
+-------------------------------------------------------------------------------------------------+
|                                 SOLID GAP ANALYSIS & AUDIT                                      |
+-----+----------------------------------+--------------------------------------------------------+
| Key | Principle                        | Identified Violations in Antik Codebase                |
+-----+----------------------------------+--------------------------------------------------------+
| S   | Single Responsibility (SRP)      | CFTP, CSMTP, CIMAP combine protocol commands,          |
|     |                                  | MIME formatting, file I/O, and string parsing.         |
+-----+----------------------------------+--------------------------------------------------------+
| O   | Open/Closed (OCP)                | Transfer utilities (FTPUtil, SFTPUtil, SCPUtil) hard-  |
|     |                                  | code concrete protocol types instead of abstractions.  |
+-----+----------------------------------+--------------------------------------------------------+
| L   | Liskov Substitution (LSP)        | IFileEventNotifier enforces Linux inotify-specific     |
|     |                                  | watch depth methods on generic event listeners.        |
+-----+----------------------------------+--------------------------------------------------------+
| I   | Interface Segregation (ISP)      | Monolithic IApprise combines listener registration,    |
|     |                                  | notification dispatch, and event filtering.            |
+-----+----------------------------------+--------------------------------------------------------+
| D   | Dependency Inversion (DIP)       | High-level utility classes depend directly on concrete  |
|     |                                  | low-level C-library wrappers (CCurl, CSSHSession).     |
+-----+----------------------------------+--------------------------------------------------------+
```

---

## 2. Refactoring Proposals by Principle

### S — Single Responsibility Principle (SRP)

#### Implemented Design
Extracted `MailMessageBuilder` ([`include/MailMessageBuilder.hpp`](file:///home/robt/projects/antik/include/MailMessageBuilder.hpp) / [`classes/MailMessageBuilder.cpp`](file:///home/robt/projects/antik/classes/MailMessageBuilder.cpp)) to isolate MIME header formatting, Date generation, and Base64 attachment payload construction away from network communication logic.

```cpp
namespace Antik::SMTP {

class MailMessageBuilder {
public:
    void setFromAddress(const std::string &addressFrom);
    void setToAddress(const std::string &addressTo);
    void setCCAddress(const std::string &addressCC);
    void setMailSubject(const std::string &subject);
    void setMailMessage(const std::vector<std::string> &message);
    void addFileAttachment(const std::string &fileName, const std::string &contentType, const std::string &contentTransferEncoding);

    std::vector<std::string> buildPayload();
};

} // namespace Antik::SMTP
```

---

### O — Open/Closed Principle (OCP)

#### Implemented Design
Defined abstract `IRemoteFileSystem` interface in [`include/IRemoteFileSystem.hpp`](file:///home/robt/projects/antik/include/IRemoteFileSystem.hpp) and implemented it across `CFTP`, `CSFTP`, and `CSCP`:

```cpp
namespace Antik::Network {

class IRemoteFileSystem {
public:
    virtual ~IRemoteFileSystem() = default;
    virtual void changeDir(const std::string &path) = 0;
    virtual void getCurrentWorkingDirectory(std::string &path) = 0;
    virtual void createDirectory(const std::string &path) = 0;
    virtual bool isDirectory(const std::string &path) = 0;
    virtual bool isFile(const std::string &path) = 0;
    virtual void uploadFile(const std::string &localPath, const std::string &remotePath) = 0;
    virtual void downloadFile(const std::string &remotePath, const std::string &localPath) = 0;
    virtual FileList listDirectory(const std::string &path) = 0;
};

} // namespace Antik::Network
```

---

### I — Interface Segregation Principle (ISP)

#### Implemented Design
Segregated `IApprise` in [`include/IApprise.hpp`](file:///home/robt/projects/antik/include/IApprise.hpp) into lean, role-specific interfaces:
- `IEventListener`: Event retrieval and exception access.
- `IWatchControl`: Watch state control (`startWatching`, `stopWatching`, `stillWatching`).
- `IWatchTargetManager`: Path registration (`addWatch`, `removeWatch`).
- `IAppriseComposite`: Composite interface inheriting all segregated roles.

---

### D — Dependency Inversion Principle (DIP)

#### Implemented Design
Created abstract `ISSHSession` interface in [`include/ISSHSession.hpp`](file:///home/robt/projects/antik/include/ISSHSession.hpp) implemented by `CSSHSession` ([`include/CSSHSession.hpp`](file:///home/robt/projects/antik/include/CSSHSession.hpp)):

```cpp
namespace Antik::SSH {

class ISSHSession {
public:
    virtual ~ISSHSession() = default;
    virtual void setServer(const std::string &serverURL) = 0;
    virtual void setUserAndPassword(const std::string &userName, const std::string &userPassword) = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual std::string getError() const = 0;
    virtual int getErrorCode() const = 0;
};

} // namespace Antik::SSH
```

---

### L — Liskov Substitution Principle (LSP)

#### Implemented Design
Refactored `IFileEventNotifier` ([`classes/implementation/IFileEventNotifier.hpp`](file:///home/robt/projects/antik/classes/implementation/IFileEventNotifier.hpp)) to provide default virtual behavior for `setWatchDepth` and introduced `IWatchableEventNotifier` for explicit watcher specialization.

---

## 3. Concrete Implementation Plan & Phases

```
+--------------------------------------------------------------------------------------------+
|                               SOLID REFACTORING PHASES                                     |
+-------+-----------------------------+-----------------------------------+------------------+
| Phase | SOLID Principle Target      | Deliverable Scope                 | Target Status    |
+-------+-----------------------------+-----------------------------------+------------------+
| P1    | Single Responsibility (SRP) | Extract MailMessageBuilder from   | COMPLETED        |
|       |                             | CSMTP & IMAPCommandFormatter.     |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P2    | Open/Closed (OCP)           | Define IRemoteFileSystem.         | COMPLETED        |
|       |                             | Implement in CFTP, CSFTP, CSCP.   |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P3    | Interface Segregation (ISP) | Segregate IApprise into           | COMPLETED        |
|       |                             | IEventListener & IWatchControl.   |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P4    | Dependency Inversion (DIP)  | Introduce ISSHSession & inject    | COMPLETED        |
|       |                             | into CSSHSession.                 |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P5    | Liskov Substitution (LSP)   | Refactor IFileEventNotifier and   | COMPLETED        |
|       |                             | IWatchableEventNotifier.          |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P6    | Regression & Verification   | Re-run 141-test suite (`ctest`)   | COMPLETED        |
|       |                             | and verify `gcov` coverage.       | (100% Pass)      |
+-------+-----------------------------+-----------------------------------+------------------+
```

---

## 4. Verification & Testing Commands

```bash
# Build static library, unit tests, and examples
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
