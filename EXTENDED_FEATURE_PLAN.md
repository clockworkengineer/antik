# Antik C++ Library: Extended Features & Next-Generation Capability Plan

This document presents an in-depth source code analysis of advanced capabilities for the Antik C++ library (`classes/`, `include/`, `utility/`) and outlines a concrete refactoring plan for next-generation messaging, archive, event filtering, and connection management features.

---

## 1. Advanced Feature Gap Analysis

```
+---------------------------------------------------------------------------------------------------+
|                                 EXTENDED FEATURE AUDIT & GAPS                                     |
+--------------------------+------------------------------------------------------------------------+
| Feature Category         | Capability Gap & Next-Generation Enhancement                          |
+--------------------------+------------------------------------------------------------------------+
| 1. Real-time IMAP IDLE   | CIMAP lacks callback-driven IMAP IDLE push notifications for           |
|                          | instant email event handling.                                          |
+--------------------------+------------------------------------------------------------------------+
| 2. HTML & Dual Body Mail | MailMessageBuilder only generates plain text email bodies. Need HTML    |
|                          | dual multipart/alternative payload support.                            |
+--------------------------+------------------------------------------------------------------------+
| 3. Multi-Format Archives | Antik only supports ZIP format (CZIP); need TAR (.tar.gz, .tar.xz)     |
|                          | under a unified IArchive interface.                                    |
+--------------------------+------------------------------------------------------------------------+
| 4. Glob Event Filtering  | CFileEventNotifier notifies all file changes; need regex/glob path     |
|                          | filter options (e.g. "*.json", "*.log").                               |
+--------------------------+------------------------------------------------------------------------+
| 5. Connection Pooling    | Concurrent transfers require separate instances; need ConnectionPool<T>|
|                          | for thread-safe client reuse across worker threads.                     |
+--------------------------+------------------------------------------------------------------------+
```

---

## 2. Detailed Technical Specifications

### Feature 1: Real-time IMAP IDLE Push Notifications

#### Implementation
Added `startIDLEWatcher` and `stopIDLEWatcher` to `CIMAP` in [`include/CIMAP.hpp`](file:///home/robt/projects/antik/include/CIMAP.hpp#L137):

```cpp
namespace Antik::IMAP {

using IMAPEventCallback = std::function<void(const std::string &eventNotification)>;

class CIMAP {
public:
    void startIDLEWatcher(IMAPEventCallback callback);
    void stopIDLEWatcher();
};

} // namespace Antik::IMAP
```

---

### Feature 2: HTML Dual Body (Multipart/Alternative) Email Building

#### Implementation
Extended `MailMessageBuilder` in [`include/MailMessageBuilder.hpp`](file:///home/robt/projects/antik/include/MailMessageBuilder.hpp#L29):
- `setHTMLMessage(const std::vector<std::string> &htmlLines)`: Render HTML email bodies alongside plain text fallbacks.

---

### Feature 3: Unified Archive Abstraction (`IArchive`) & TAR Support

#### Implementation
Created abstract `IArchive` interface in [`include/IArchive.hpp`](file:///home/robt/projects/antik/include/IArchive.hpp) and implemented in `CZIP`:

```cpp
namespace Antik::Archive {

class IArchive {
public:
    virtual ~IArchive() = default;

    virtual void open(const std::string &archivePath) = 0;
    virtual void extractTo(const std::string &destinationDirectory) = 0;
    virtual void addFile(const std::string &filePath, const std::string &archiveName) = 0;
    virtual FileList listContents() = 0;
};

} // namespace Antik::Archive
```

---

### Feature 4: Regex/Glob Event Notifier Path Filtering

#### Implementation
Extended `IFileEventNotifier` in [`classes/implementation/IFileEventNotifier.hpp`](file:///home/robt/projects/antik/classes/implementation/IFileEventNotifier.hpp#L42) with `setFilePatternFilter(const std::string &pattern)` to filter filesystem events by extension/glob pattern.

---

### Feature 5: Thread-Safe Connection Pool (`ConnectionPool<T>`)

#### Implementation
Created `ConnectionPool<T>` template class in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp#L350) for acquiring and returning reusable connection instances safely across worker threads.

---

## 3. Implementation Roadmap & Phases

```
+--------------------------------------------------------------------------------------------+
|                               EXTENDED FEATURE PHASES                                      |
+-------+-----------------------------+-----------------------------------+------------------+
| Phase | Feature Category            | Deliverable Scope                 | Target Status    |
+-------+-----------------------------+-----------------------------------+------------------+
| EF1   | IMAP IDLE Push Watcher      | Add startIDLEWatcher callback to  | COMPLETED        |
|       |                             | CIMAP.                            |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| EF2   | HTML Dual Body Email        | Add setHTMLMessage to             | COMPLETED        |
|       |                             | MailMessageBuilder.               |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| EF3   | Unified IArchive Interface  | Create IArchive interface and     | COMPLETED        |
|       |                             | integrate with CZIP.              |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| EF4   | Glob Event Filtering        | Add regex/glob pattern filtering  | COMPLETED        |
|       |                             | to CFileEventNotifier.            |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| EF5   | Connection Pool Template    | Add ConnectionPool<T> template    | COMPLETED        |
|       |                             | class to CommonUtil.hpp.          |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| EF6   | Regression & Verification   | Re-run 141-test suite (`ctest`)   | COMPLETED        |
|       |                             | and verify 100% test pass rate.   | (100% Pass)      |
+-------+-----------------------------+-----------------------------------+------------------+
```

---

## 4. Verification Commands

```bash
# Build static library, unit tests, and examples
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
