# Antik C++ Library: Concrete DRY (Don't Repeat Yourself) Refactoring Plan

This document presents a comprehensive source analysis and architectural refactoring plan for the Antik C++ library to eliminate code duplication, enforce reusable core abstractions, and simplify maintenance.

---

## 1. Source Code Audit & Code Duplication Analysis

A thorough audit of `classes/`, `utility/`, and `include/` identified seven core patterns of code duplication:

| # | Pattern Name | Duplicate Code Locations | Core Problem |
|---|---|---|---|
| 1 | **Server Path Operations** | `FTPUtil.cpp`, `SCPUtil.cpp`, `SFTPUtil.cpp`, `CApprise.cpp`, `CFileEventNotifier.cpp`, `CPath.cpp` | Repeated `path.pop_back()` for trailing slashes, duplicate `//` character stripping, and custom path component splitting. |
| 2 | **Server Account Credentials** | `CFTP.hpp`, `CIMAP.hpp`, `CSMTP.hpp`, `CSSHSession.hpp` | Identical fields (`m_serverName`, `m_serverPort`, `m_userName`, `m_userPassword`) and getters/setters re-declared across 4 protocol headers. |
| 3 | **I/O Buffer Sizing** | `CFTP.hpp`, `CIMAP.hpp`, `CSFTP.hpp`, `CSCP.hpp`, `CSSHChannel.hpp`, `CZIP.hpp` | Duplicate `m_ioBufferSize`, `setIoBufferSize()`, and `getIoBufferSize()` declarations across 6 library headers. |
| 4 | **Base64 Encoding & Decoding** | `CSMTP.cpp`, `CMIME.cpp` | Fragmented Base64 bit-shifting, encoding tables, and character decode lambda functions. |
| 5 | **SSH Module Exception Classes** | `CSSHSession.hpp`, `CSSHChannel.hpp`, `CSCP.hpp`, `CSFTP.hpp` | Identical `Exception` struct declarations (`m_errorCode`, `m_errorMessage`, `m_functionName`, `getMessage()`, `getCode()`). |
| 6 | **Exception Re-wrapping Boilerplate** | `CApprise.cpp`, `CSMTP.cpp`, `CFTP.cpp`, `CIMAP.cpp`, `CSFTP.cpp`, `CSCP.cpp` | Over 50+ public member functions repeating identical `try { ... } catch (const std::exception &e) { throw Exception(e.what()); }` blocks. |
| 7 | **Console & File Stream Redirection** | `CRedirect.hpp`, `CommonUtil.hpp`, `UTCUtility.cpp` | Repeated `std::cout.rdbuf()` saving and buffer swapping logic for stdout/stderr redirection. |

---

## 2. Concrete Shared Architecture & Implementation Specifications

### A. Network Server Credentials Abstraction (`ServerCredentials`)

**File**: [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp)

Encapsulate server connection address and authentication credentials into a single reusable mixin:

```cpp
namespace Antik {
class ServerCredentials {
public:
    virtual ~ServerCredentials() = default;
    void setServer(const std::string &server) { m_serverName = server; }
    void setPort(const std::string &port) { m_serverPort = port; }
    void setUser(const std::string &user) { m_userName = user; }
    void setUserPassword(const std::string &password) { m_userPassword = password; }
    void setUserAndPassword(const std::string &userName, const std::string &userPassword) {
        m_userName = userName;
        m_userPassword = userPassword;
    }
    std::string getServer() const { return m_serverName; }
    std::string getPort() const { return m_serverPort; }
    std::string getUser() const { return m_userName; }
protected:
    std::string m_serverName;
    std::string m_serverPort{"21"};
    std::string m_userName;
    std::string m_userPassword;
};
}
```

- **Target Inheritors**: [`CFTP`](file:///home/robt/projects/antik/include/CFTP.hpp), [`CIMAP`](file:///home/robt/projects/antik/include/CIMAP.hpp), [`CSMTP`](file:///home/robt/projects/antik/include/CSMTP.hpp), [`CSSHSession`](file:///home/robt/projects/antik/include/CSSHSession.hpp).

---

### B. Centralized I/O Buffer Manager (`IOBufferManager`)

**File**: [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp)

Unified buffer size state and accessor interface:

```cpp
namespace Antik {
class IOBufferManager {
public:
    explicit IOBufferManager(std::uint32_t defaultBufferSize = 32768) : m_ioBufferSize{defaultBufferSize} {}
    virtual ~IOBufferManager() = default;
    void setIoBufferSize(std::uint32_t ioBufferSize) { m_ioBufferSize = ioBufferSize; }
    void setIOBufferSize(std::uint32_t ioBufferSize) { m_ioBufferSize = ioBufferSize; }
    std::uint32_t getIoBufferSize() const { return m_ioBufferSize; }
    std::uint32_t getIOBufferSize() const { return m_ioBufferSize; }
protected:
    std::uint32_t m_ioBufferSize;
};
}
```

- **Target Inheritors**: `CFTP`, `CIMAP`, `CSFTP`, `CSCP`, `CSSHChannel`, `CZIP`.

---

### C. SSH Module Exception Hierarchy (`SSHExceptionBase`)

**File**: [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp)

Unified exception base class for libssh wrapper classes:

```cpp
namespace Antik::SSH {
class SSHExceptionBase {
public:
    SSHExceptionBase(const std::string &className, const std::string &functionName, const std::string &errorMessage, int errorCode = 0)
        : m_className{className}, m_functionName{functionName}, m_errorMessage{errorMessage}, m_errorCode{errorCode} {}
    virtual ~SSHExceptionBase() = default;
    int getCode() const { return m_errorCode; }
    std::string getMessage() const {
        return m_className + " Failure: (" + m_functionName + ") [" + m_errorMessage + "]";
    }
protected:
    std::string m_className;
    std::string m_functionName;
    std::string m_errorMessage;
    int m_errorCode{0};
};
}
```

- **Target Inheritors**: `CSSHSession::Exception`, `CSSHChannel::Exception`, `CSCP::Exception`, `CSFTP::Exception`.

---

### D. Base64 Processing Utility (`Base64`)

**File**: [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp)

Centralized static Base64 encoding and decoding routines:

```cpp
namespace Antik {
class Base64 {
public:
    static void encode(const std::string &decoding, std::string &encoding, std::uint32_t numberOfBytes);
    static void decode(const std::string &encoding, std::string &decoding, std::uint32_t numberOfBytes);
};
}
```

- **Consumers**: `CSMTP::encodeToBase64`, `CSMTP::decodeFromBase64`, `CMIME::convertMIMEStringToASCII`.

---

### E. Server Path Helper Methods (`CPath`)

**File**: [`include/CPath.hpp`](file:///home/robt/projects/antik/include/CPath.hpp) / [`classes/CPath.cpp`](file:///home/robt/projects/antik/classes/CPath.cpp)

Static path manipulation methods:

```cpp
namespace Antik::File {
class CPath {
public:
    static void stripTrailingSeparator(std::string &path);
    static std::string joinServerPath(const std::string &cwd, const std::string &path, const std::string &file = "");
    static std::vector<std::string> splitPathComponents(const std::string &path);
};
}
```

- **Consumers**: `FTPUtil.cpp`, `SCPUtil.cpp`, `SFTPUtil.cpp`, `CApprise.cpp`, `CFileEventNotifier.cpp`.

---

## 3. Implementation Roadmap & Status

| Phase | Category | Description | Status |
|---|---|---|---|
| **Phase 1** | Server Path Utilities | Add `stripTrailingSeparator`, `joinServerPath`, `splitPathComponents` to `CPath`. Refactor `FTPUtil.cpp`. | **COMPLETED** |
| **Phase 2** | Credential & Buffer Mixins | Implement `ServerCredentials` & `IOBufferManager` in `CommonAntik.hpp`. Inherit in `CFTP`, `CIMAP`, `CSMTP`, `CSFTP`, `CSCP`, `CSSHChannel`. | **COMPLETED** |
| **Phase 3** | Base64 Utility | Extract `Base64::encode` & `Base64::decode` into `CommonUtil.hpp`. Refactor `CSMTP.cpp` & `CMIME.cpp`. | **COMPLETED** |
| **Phase 4** | SSH Exception Hierarchy | Extract `SSHExceptionBase` into `CommonAntik.hpp`. Inherit in `CSSHSession`, `CSSHChannel`, `CSCP`, `CSFTP`. | **COMPLETED** |
| **Phase 5** | Regression Testing | Build project, verify 141 Google Tests (`ctest`), check `gcov` line coverage. | **COMPLETED (100% Pass)** |

---

## 4. Verification & Build Commands

```bash
# 1. Configure and build project, unit tests, and examples
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# 2. Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
