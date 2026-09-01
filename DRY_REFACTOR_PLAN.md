# Antik C++ Library: Concrete DRY (Don't Repeat Yourself) Refactor Plan

This document outlines a technical plan for refactoring the Antik C++ library to eliminate code duplication, improve maintainability, reduce boilerplate, and establish reusable core abstractions across network, file, and utility modules.

---

## 1. Executive Summary & Codebase Audit Findings

A comprehensive audit of the Antik codebase identified several primary areas of code duplication and redundant boilerplate:

1. **Path Construction & Normalization**: Identical trailing slash removal (`path.pop_back()`), double slash consolidation (`find("//")`), and server path joining logic duplicated across `FTPUtil.cpp`, `SCPUtil.cpp`, `SFTPUtil.cpp`, `CApprise.cpp`, `CFileEventNotifier.cpp`, and `CPath.cpp`.
2. **Exception Handling Boilerplate**: Virtually every public method in `CApprise.cpp`, `CSMTP.cpp`, `CFTP.cpp`, `CIMAP.cpp`, `CSFTP.cpp`, and `CSCP.cpp` duplicates `try { ... } catch (const std::exception &e) { throw Exception(e.what()); }`.
3. **Server Account Credentials**: Identical data members (`m_userName`, `m_userPassword`, `m_serverName`, `m_serverPort`) and accessors (`setUserAndPassword`, `setServer`, `getUser`, `getServer`, `setPort`) duplicated in `CFTP`, `CIMAP`, `CSMTP`, and `CSSHSession`.
4. **I/O Buffer Management**: `m_ioBufferSize`, `setIoBufferSize()`, `getIoBufferSize()`, and default buffer initialization logic duplicated in `CFTP`, `CIMAP`, `CSCP`, `CSFTP`, `CSSHChannel`, and `CZIP`.
5. **Base64 Utility Duplication**: Base64 encoding/decoding implementations fragmented across `CSMTP.cpp` and `CMIME.cpp`.

---

## 2. Refactoring Components & Technical Specifications

### Component A: Path & Server Path Utility (`CPath` & `CommonUtil`)

#### Problem
`constructRemotePathName`, trailing-slash removal, and path component splitting (`boost::split` on `/`) are re-implemented in multiple utility files:
- `FTPUtil.cpp` (lines 51–60)
- `SCPUtil.cpp` (lines 43–47)
- `SFTPUtil.cpp` (lines 110–125)
- `CApprise.cpp` (lines 70–75)
- `CFileEventNotifier.cpp` (lines 124–128)

#### Target Refactor
Extend [`include/CPath.hpp`](file:///home/robt/projects/antik/include/CPath.hpp) and [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp) with centralized, reusable helper methods:

```cpp
namespace Antik::File {
class CPath {
public:
    static void stripTrailingSeparator(std::string &path);
    static std::string joinServerPath(const std::string &base, const std::string &sub, const std::string &file = "");
    static std::string normalizeServerPath(const std::string &path);
    static std::vector<std::string> splitPathComponents(const std::string &path);
};
}
```

---

### Component B: Standardized Exception Handling (`CommonAntik.hpp`)

#### Problem
Over 50+ public methods repeat the exact same try-catch block:
```cpp
try {
    // method body
} catch (const std::exception &e) {
    throw Exception(e.what());
}
```

#### Target Refactor
Introduce a generic exception wrapper template in [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp):

```cpp
namespace Antik {
template <typename ExType, typename Func>
auto executeWithExceptionWrapper(const std::string &contextPrefix, Func &&func) {
    try {
        return func();
    } catch (const std::exception &e) {
        throw ExType(contextPrefix.empty() ? e.what() : contextPrefix + ": " + e.what());
    }
}
}
```

This reduces 7-line try/catch blocks across `CApprise.cpp`, `CSMTP.cpp`, `CFTP.cpp`, etc., down to single clean expressions.

---

### Component C: Reusable Server Credentials Mixin (`ServerCredentials`)

#### Problem
`CFTP`, `CIMAP`, `CSMTP`, and `CSSHSession` each implement identical fields:
- `std::string m_userName`, `m_userPassword`, `m_serverName`, `m_serverPort`
- Methods: `setUserAndPassword`, `setServer`, `getUser`, `getServer`, `setPort`, `getPort`

#### Target Refactor
Extract a reusable `ServerCredentials` class in [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp) or [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp):

```cpp
namespace Antik::Network {
class ServerCredentials {
public:
    void setServer(const std::string &server) { m_serverName = server; }
    void setPort(const std::string &port) { m_serverPort = port; }
    void setUserAndPassword(const std::string &user, const std::string &pass) {
        m_userName = user;
        m_userPassword = pass;
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

---

### Component D: Centralized IO Buffer Manager (`IOBufferManager`)

#### Problem
Buffer size tracking and management (`m_ioBufferSize`, `setIoBufferSize`, `getIoBufferSize`) is declared 6 times across different header files (`CFTP.hpp`, `CIMAP.hpp`, `CSCP.hpp`, `CSFTP.hpp`, `CSSHChannel.hpp`, `CZIP.hpp`).

#### Target Refactor
Create `IOBufferManager` in [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp):

```cpp
namespace Antik {
class IOBufferManager {
public:
    explicit IOBufferManager(std::uint32_t defaultSize = 32768) : m_ioBufferSize{defaultSize} {}
    void setIoBufferSize(std::uint32_t size) { m_ioBufferSize = size; }
    std::uint32_t getIoBufferSize() const { return m_ioBufferSize; }
protected:
    std::uint32_t m_ioBufferSize;
};
}
```

---

### Component E: Base64 Encoding Consolidation (`Base64Util`)

#### Problem
`CSMTP::encodeToBase64` / `CSMTP::decodeFromBase64` and `CMIME::convertMIMEStringToASCII` both contain Base64 conversion code.

#### Target Refactor
Create a standalone `Base64Util` helper class in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp) or [`include/CMIME.hpp`](file:///home/robt/projects/antik/include/CMIME.hpp):

```cpp
namespace Antik::Util {
class Base64 {
public:
    static std::string encode(const std::string &data);
    static std::string decode(const std::string &base64Str);
};
}
```

---

## 3. Implementation Phases & Verification Strategy

| Phase | Target Components | Description & Expected Outcome |
|---|---|---|
| **Phase 1** | `CPath`, `CommonUtil` | Add `stripTrailingSeparator`, `joinServerPath`, `splitPathComponents` to `CPath`. Refactor `FTPUtil`, `SCPUtil`, `SFTPUtil`, `CApprise`. |
| **Phase 2** | `CommonAntik.hpp` | Implement `ServerCredentials` & `IOBufferManager`. Refactor `CFTP`, `CIMAP`, `CSMTP`, `CSCP`, `CSFTP`, `CSSHChannel`. |
| **Phase 3** | `Base64` Consolidation | Extract `Base64` utility methods into `CommonUtil`. Refactor `CSMTP` & `CMIME`. |
| **Phase 4** | Exception Wrapper | Implement `executeWithExceptionWrapper` and refactor `CApprise.cpp` and `CSMTP.cpp`. |
| **Phase 5** | Regression Testing | Re-run full 141-test Google Test suite (`ctest`) and verify code coverage metrics (`gcov`). |

---

## 4. Verification Commands

To verify that the refactored code maintains 100% functionality and test coverage:

```bash
# 1. Configure and build project with tests and examples
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# 2. Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure

# 3. Check gcov coverage report
gcov -o build/CMakeFiles/antik.dir/classes/*.cpp.o classes/*.cpp
```
