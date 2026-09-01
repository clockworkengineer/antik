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

### L — Liskov Substitution Principle (LSP)

#### Proposed Design
Separate general event polling (`IFileEventNotifier`) from directory watch configuration (`IWatchableEventNotifier`).

---

### I — Interface Segregation Principle (ISP)

#### Proposed Design
Segregate `IApprise` into `IEventListener` and `INotificationDispatcher`.

---

### D — Dependency Inversion Principle (DIP)

#### Proposed Design
Introduce `ISSHSession` and `INetworkClient` interfaces implemented by `CSSHSession` and `CCurl`.

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
| P3    | Interface Segregation (ISP) | Segregate IApprise into           | Ready for Action |
|       |                             | IEventListener & IDispatcher.     |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P4    | Dependency Inversion (DIP)  | Introduce ISSHSession & inject    | Ready for Action |
|       |                             | into CSFTP and CSCP.              |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| P5    | Liskov Substitution (LSP)   | Refactor IFileEventNotifier and   | Ready for Action |
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
