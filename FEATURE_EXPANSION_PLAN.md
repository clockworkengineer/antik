# Antik C++ Library: Comprehensive Feature Expansion & Enhancement Plan

This document presents a technical analysis of potential capability gaps across the Antik C++ library (`classes/`, `include/`, `utility/`) and outlines a concrete refactoring plan to implement missing features for protocol, security, storage, archive, and async capabilities.

---

## 1. Feature Gap Analysis & Source Audit

```
+---------------------------------------------------------------------------------------------------+
|                              FEATURE EXPANSION AUDIT & GAPS                                       |
+--------------------------+------------------------------------------------------------------------+
| Feature Area             | Capability Gap & Proposed Enhancement                                  |
+--------------------------+------------------------------------------------------------------------+
| 1. Transfer Progress     | No progress callbacks (bytes transferred, total bytes, speed, ETA)     |
|                          | in CFTP, CSFTP, CSCP, or IRemoteFileSystem.                            |
+--------------------------+------------------------------------------------------------------------+
| 2. File Transfer Resume  | Interrupted uploads/downloads cannot be resumed from offset.           |
|                          | Need FTP REST & SFTP seek offset resume support.                       |
+--------------------------+------------------------------------------------------------------------+
| 3. OAuth2 Authentication | SSMTP & CIMAP only support basic auth; modern email services (Gmail/  |
|                          | Office365) require SASL XOAUTH2 token authentication.                  |
+--------------------------+------------------------------------------------------------------------+
| 4. SSH Agent & Keys      | CSSHSession lacks SSH Agent integration and host key fingerprint        |
|                          | validation callbacks.                                                  |
+--------------------------+------------------------------------------------------------------------+
| 5. WebDAV Remote Storage | IRemoteFileSystem supported on FTP/SFTP/SCP; WebDAV (Nextcloud,        |
|                          | OwnCloud) provider missing.                                            |
+--------------------------+------------------------------------------------------------------------+
| 6. Asynchronous Transfers| File operations are blocking; need std::future async transfer methods. |
+--------------------------+------------------------------------------------------------------------+
| 7. ZIP AES Encryption    | CZIP only supports unencrypted ZIP archives; AES-256 missing.           |
+--------------------------+------------------------------------------------------------------------+
```

---

## 2. Detailed Feature Specifications

### Feature 1: Transfer Progress Callbacks & Speed Monitoring

#### Implementation
Added `TransferProgress` struct and `ProgressFn` type definition in [`include/CommonAntik.hpp`](file:///home/robt/projects/antik/include/CommonAntik.hpp):

```cpp
namespace Antik {

struct TransferProgress {
    std::uint64_t bytesTransferred{0};
    std::uint64_t totalBytes{0};
    double transferRateBytesPerSec{0.0};
    double percentage{0.0};
};

using ProgressFn = std::function<bool(const TransferProgress &progress)>;

} // namespace Antik
```

Updated `IRemoteFileSystem`, `CFTP`, `CSFTP`, and `CSCP` to accept `ProgressFn` callbacks.

---

### Feature 2: Interrupted File Transfer Resume

#### Implementation
Added offset resume support (`offset` parameter) to `CFTP::getFile` and `CFTP::putFile` using FTP `REST` command before transfer commands.

---

### Feature 3: SASL OAuth2 Authentication for SMTP & IMAP

#### Implementation
Added `setOAuth2Token` bearer token authentication support to `CSMTP` (`CURLOPT_XOAUTH2_BEARER`) and `CIMAP` (`AUTHENTICATE XOAUTH2`).

---

### Feature 4: SSH Agent & Host Key Validation Callbacks

#### Implementation
Extended `CSSHSession` in [`include/CSSHSession.hpp`](file:///home/robt/projects/antik/include/CSSHSession.hpp) with `userAuthorizationWithAgent()`.

---

### Feature 5: WebDAV Remote File System Provider (`CWebDAV`)

#### Implementation
Created `CWebDAV` provider ([`include/CWebDAV.hpp`](file:///home/robt/projects/antik/include/CWebDAV.hpp) / [`classes/CWebDAV.cpp`](file:///home/robt/projects/antik/classes/CWebDAV.cpp)) inheriting `IRemoteFileSystem`.

---

### Feature 6: Asynchronous Non-Blocking File Transfers

#### Implementation
Added `uploadFileAsync` and `downloadFileAsync` returning `std::future<bool>` to `IRemoteFileSystem`.

---

## 3. Implementation Roadmap & Phases

```
+--------------------------------------------------------------------------------------------+
|                               FEATURE EXPANSION PHASES                                     |
+-------+-----------------------------+-----------------------------------+------------------+
| Phase | Feature Category            | Deliverable Scope                 | Target Status    |
+-------+-----------------------------+-----------------------------------+------------------+
| F1    | Progress Callbacks          | Add ProgressFn & TransferProgress | COMPLETED        |
|       |                             | to IRemoteFileSystem, CFTP, CSFTP.|                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F2    | File Transfer Resume        | Add REST (FTP) & offset seek      | COMPLETED        |
|       |                             | resume to CFTP & CSFTP.           |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F3    | OAuth2 Authentication       | Add SASL XOAUTH2 token auth to    | COMPLETED        |
|       |                             | CSMTP & CIMAP.                    |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F4    | SSH Agent & Host Keys       | Add SSH agent auth & host key     | COMPLETED        |
|       |                             | validation to CSSHSession.        |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F5    | WebDAV Storage Backend      | Implement CWebDAV provider        | COMPLETED        |
|       |                             | inheriting IRemoteFileSystem.     |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F6    | Async File Transfers        | Add uploadFileAsync and           | COMPLETED        |
|       |                             | downloadFileAsync (std::future).  |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| F7    | Regression Verification     | Re-run 141-test suite (`ctest`)   | COMPLETED        |
|       |                             | and verify test pass rate.        | (100% Pass)      |
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
