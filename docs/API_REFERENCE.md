# Antik C++ Library: API Reference Manual

This document provides a comprehensive API reference for public classes and interfaces in the **Antik C++ Library**.

---

## 1. Network & Remote File Systems (`Antik::Network`)

### `IRemoteFileSystem` Interface
Defined in [`include/IRemoteFileSystem.hpp`](file:///home/robt/projects/antik/include/IRemoteFileSystem.hpp).

```cpp
virtual void changeDir(const std::string &path) = 0;
virtual void getCurrentWorkingDirectory(std::string &path) = 0;
virtual void createDirectory(const std::string &path) = 0;
virtual bool isDirectory(const std::string &path) = 0;
virtual bool isFile(const std::string &path) = 0;
virtual void uploadFile(const std::string &localPath, const std::string &remotePath, ProgressFn progressFn = nullptr) = 0;
virtual void downloadFile(const std::string &remotePath, const std::string &localPath, ProgressFn progressFn = nullptr) = 0;
virtual FileList listDirectory(const std::string &path) = 0;
virtual std::future<bool> uploadFileAsync(const std::string &localPath, const std::string &remotePath, ProgressFn progressFn = nullptr);
virtual std::future<bool> downloadFileAsync(const std::string &remotePath, const std::string &localPath, ProgressFn progressFn = nullptr);
```

### `CFTP` Class
Defined in [`include/CFTP.hpp`](file:///home/robt/projects/antik/include/CFTP.hpp). Inherits `IRemoteFileSystem` and `ServerCredentials`.
- `std::uint16_t getFile(const std::string &remotePath, const std::string &localPath, std::size_t offset = 0)`
- `std::uint16_t putFile(const std::string &remotePath, const std::string &localPath, std::size_t offset = 0)`
- `void setPassiveTransferMode(bool passiveEnabled)`

### `CWebDAV` Class
Defined in [`include/CWebDAV.hpp`](file:///home/robt/projects/antik/include/CWebDAV.hpp). Inherits `IRemoteFileSystem` and `ServerCredentials`.

---

## 2. Email & Messaging (`Antik::SMTP` & `Antik::IMAP`)

### `MailMessageBuilder` Class
Defined in [`include/MailMessageBuilder.hpp`](file:///home/robt/projects/antik/include/MailMessageBuilder.hpp).
- `void setFromAddress(const std::string &addressFrom)`
- `void setToAddress(const std::string &addressTo)`
- `void setCCAddress(const std::string &addressCC)`
- `void setMailSubject(const std::string &subject)`
- `void setMailMessage(const std::vector<std::string> &message)`
- `void setHTMLMessage(const std::vector<std::string> &htmlMessage)`
- `void addFileAttachment(const std::string &fileName, const std::string &contentType, const std::string &encoding)`
- `std::vector<std::string> buildPayload()`

### `CSMTP` Class
Defined in [`include/CSMTP.hpp`](file:///home/robt/projects/antik/include/CSMTP.hpp).
- `void setServer(const std::string &serverURL)`
- `void setUserAndPassword(const std::string &user, const std::string &password)`
- `void setOAuth2Token(const std::string &user, const std::string &bearerToken)`
- `void postMail()`

### `CIMAP` Class
Defined in [`include/CIMAP.hpp`](file:///home/robt/projects/antik/include/CIMAP.hpp).
- `void connect()`
- `void setOAuth2Token(const std::string &user, const std::string &bearerToken)`
- `std::string sendCommand(const std::string &commandLine)`
- `void startIDLEWatcher(IMAPEventCallback callback)`

---

## 3. SSH Protocol (`Antik::SSH`)

### `ISSHSession` Interface
Defined in [`include/ISSHSession.hpp`](file:///home/robt/projects/antik/include/ISSHSession.hpp).
- `void setServer(const std::string &serverURL)`
- `void setUserAndPassword(const std::string &user, const std::string &password)`
- `void connect()`
- `void disconnect()`

### `CSSHSession` Class
Defined in [`include/CSSHSession.hpp`](file:///home/robt/projects/antik/include/CSSHSession.hpp). Inherits `ISSHSession`.
- `int userAuthorizationWithAgent()`

---

## 4. Utilities (`Antik`)

### `ConnectionPool<T>` Template Class
Defined in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp).
- `std::unique_ptr<T> acquire()`
- `void release(std::unique_ptr<T> connection)`

### `StringUtil` Class
Defined in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp).
- `static std::string trim(std::string_view str)`
- `static bool startsWithIgnoreCase(std::string_view str, std::string_view prefix)`
