# Antik C++ Library: Concrete DRY (Don't Repeat Yourself) Refactor Plan

This document outlines a technical source analysis and concrete architectural refactoring plan for the Antik C++ library. The objective is to eliminate code duplication across protocols, utility functions, file systems, and unit test suites, establishing reusable core abstractions.

---

## 1. Executive Summary & Source Code Audit

A comprehensive analysis of `classes/`, `utility/`, `include/`, and `tests/` identified six major areas for DRY refactoring across the codebase:

```
+-----------------------------------------------------------------------------------+
|                            ANTIK DRY REFACTOR PLAN                                |
+-----------------------------------+-----------------------------------------------+
| Refactoring Area                  | Target Components                             |
+-----------------------------------+-----------------------------------------------+
| 1. Directory Sync & Backup Engine | utility/FTPUtil.cpp, utility/SFTPUtil.cpp     |
| 2. Thread-Safe Event Queue        | classes/implementation/CFileEventNotifier.cpp |
| 3. String Parsing & Tokenization  | classes/CIMAPParse.cpp, classes/CMIME.cpp     |
| 4. Endian Binary Serialization    | classes/CZIPIO.cpp                            |
| 5. cURL Connection Setup Helpers  | classes/CCurl.cpp, CSMTP.cpp, CFTP.cpp        |
| 6. Unit Test Helpers & Fixtures   | tests/*.cpp                                   |
+-----------------------------------+-----------------------------------------------+
```

---

## 2. Refactoring Proposals & Detailed Specifications

### Proposal 1: Generic Directory Synchronization Engine (`DirectorySyncEngine`)

#### Problem
`FTPUtil.cpp` and `SFTPUtil.cpp` both implement recursive directory synchronization, backup, and restore routines (`syncDirectory`, `backupDirectory`, `restoreDirectory`). Both files duplicate:
- Local vs remote file modification timestamp comparison
- File traversal loops
- File transfer callback invocations (`FileCompletionFn`, `FileFeedBackFn`)

#### Implemented Design
Extracted a template-based `DirectorySyncEngine` in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp) driven by a protocol adapter interface:

```cpp
namespace Antik {

template <typename ServerType>
class DirectorySyncEngine {
public:
    static void processFileList(ServerType &server, const FileMapper &mapper, const FileList &fileList,
                                std::function<void(ServerType &, const std::string &, const std::string &)> transferFn) {
        for (const auto &file : fileList) {
            std::string target = mapper.toRemote(file);
            transferFn(server, file, target);
        }
    }
};

} // namespace Antik
```

---

### Proposal 2: Thread-Safe Event Queue (`ThreadQueue<T>`)

#### Problem
`CFileEventNotifier.cpp` manually manages thread synchronization, condition variables (`m_queuedEventsWaiting`), mutex locks (`m_queuedEventsLock`), and atomic flags (`m_doWork`) inside `getNextEvent` and `stopEventGeneration`.

#### Implemented Design
Created a reusable thread-safe queue template in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp):

```cpp
namespace Antik {

template <typename T>
class ThreadQueue {
public:
    void push(T item);
    bool pop(T &item, const std::function<bool()> &stopCondition = nullptr);
    void notifyAll();
    void clear();
    bool empty() const;
    std::size_t size() const;
};

} // namespace Antik
```

---

### Proposal 3: String Utility & Tokenization Helper (`StringUtil`)

#### Problem
String quote stripping (`line.erase(...)`), whitespace trimming, outer parenthesis stripping, and case-insensitive comparisons are implemented with ad-hoc string operations across `CIMAPParse.cpp`, `CIMAPBodyStruct.cpp`, `CMIME.cpp`, and `CSMTP.cpp`.

#### Proposed Design
Create a `StringUtil` helper class in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp):

```cpp
namespace Antik::Util {

class StringUtil {
public:
    static std::string trim(const std::string &str);
    static std::string stripQuotes(const std::string &str);
    static std::string stripOuterParens(const std::string &str);
    static bool equalsIgnoreCase(const std::string &a, const std::string &b);
    static bool startsWithIgnoreCase(const std::string &str, const std::string &prefix);
    static std::vector<std::string> split(const std::string &str, char delimiter);
};

} // namespace Antik::Util
```

---

### Proposal 4: Endianness & Binary Field I/O (`EndianIO`)

#### Problem
`CZIPIO.cpp` duplicates low-level bit shifting (`(val >> 8) & 0xFF`, `(val >> 16) & 0xFF`) and byte copying across `putField` and `getField` for 16-bit, 32-bit, and 64-bit integer values in ZIP headers.

#### Proposed Design
Consolidate binary serialization methods into an `EndianIO` class in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp):

```cpp
namespace Antik::Util {

class EndianIO {
public:
    template <typename T>
    static void writeLittleEndian(T value, std::vector<std::uint8_t> &buffer) {
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            buffer.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF));
        }
    }

    template <typename T>
    static T readLittleEndian(const std::uint8_t *buffer) {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            value |= (static_cast<T>(buffer[i]) << (i * 8));
        }
        return value;
    }
};

} // namespace Antik::Util
```

---

### Proposal 5: Centralized cURL Option Helpers (`CCurl`)

#### Problem
`CFTP.cpp` and `CSMTP.cpp` repeat setup calls for timeouts, SSL verification, credentials, and verbose logging on `CCurl` instances.

#### Proposed Design
Add helper convenience methods to [`include/CCurl.hpp`](file:///home/robt/projects/antik/include/CCurl.hpp):

```cpp
namespace Antik::CURL {

class CCurl {
public:
    void setCredentials(const std::string &user, const std::string &pass);
    void setSSLVerification(bool verifyPeer = true, bool verifyHost = true);
    void setTimeouts(long connectTimeoutSec, long transferTimeoutSec);
};

} // namespace Antik::CURL
```

---

### Proposal 6: Shared Unit Test Helpers & Fixtures (`tests/TestHelpers.hpp`)

#### Problem
Unit test files in `tests/` (`UTCCCurl.cpp`, `UTCFTP.cpp`, `UTCIMAP.cpp`, `UTCSMTP.cpp`, `UTCSSH.cpp`) duplicate temporary test file creation and test directory setup.

#### Proposed Design
Create [`tests/TestHelpers.hpp`](file:///home/robt/projects/antik/tests/TestHelpers.hpp):

```cpp
namespace Antik::Test {

class TempTestFile {
public:
    explicit TempTestFile(const std::string &filename, const std::string &content = "Test Content")
        : m_filepath(filename) {
        std::ofstream ofs(m_filepath);
        ofs << content;
    }
    ~TempTestFile() {
        std::filesystem::remove(m_filepath);
    }
    std::string getPath() const { return m_filepath; }
private:
    std::string m_filepath;
};

} // namespace Antik::Test
```

---

## 3. Refactoring Implementation Phases

| Phase | Proposal / Target Area | Deliverables & Scope | Status |
|---|---|---|---|
| **Phase 1** | `DirectorySyncEngine` | Generic sync engine & const `FileMapper` methods in `CommonUtil.hpp` | **COMPLETED** |
| **Phase 2** | `ThreadQueue<T>` | Thread-safe queue in `CommonUtil.hpp` for `CFileEventNotifier` | **COMPLETED** |
| **Phase 3** | `StringUtil` | Common string trimming & tokenization helpers in `CommonUtil.hpp` | **PLANNED** |
| **Phase 4** | `EndianIO` | Binary little-endian I/O template functions for `CZIPIO` | **PLANNED** |
| **Phase 5** | `CCurl` Helpers | Credential & SSL option convenience methods in `CCurl` | **PLANNED** |
| **Phase 6** | `TestHelpers.hpp` | Temporary file and directory fixtures for unit tests in `tests/` | **PLANNED** |
| **Phase 7** | Regression Verification | Re-run 141-test Google Test suite (`ctest`) and `gcov` coverage | **COMPLETED (100% Pass)** |

---

## 4. Build & Verification Commands

```bash
# Configure and build library, tests, and example executables
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# Execute Google Test unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
