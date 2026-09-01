# Antik C++ Library: Developer Guide & Code Examples

This guide provides practical, copy-paste C++17 recipes for common scenarios using the **Antik C++ Library**.

---

## 1. Remote File Transfers via `IRemoteFileSystem`

```cpp
#include "CFTP.hpp"
#include "CSFTP.hpp"
#include "IRemoteFileSystem.hpp"
#include <iostream>

void syncFiles(Antik::Network::IRemoteFileSystem &remoteFs)
{
    // List directory contents
    Antik::FileList files = remoteFs.listDirectory("/backup");
    for (const auto &file : files)
    {
        std::cout << "Remote File: " << file << std::endl;
    }

    // Upload file with progress tracking
    remoteFs.uploadFile("local_report.pdf", "/backup/report.pdf", [](const Antik::TransferProgress &progress) {
        std::cout << "Transferred: " << progress.bytesTransferred << " / " << progress.totalBytes << " bytes (" << progress.percentage << "%)\n";
        return true; // continue transfer
    });
}
```

---

## 2. Sending Email with MIME Attachments & SASL XOAUTH2

```cpp
#include "MailMessageBuilder.hpp"
#include "CSMTP.hpp"

void sendOAuth2Email()
{
    // Build MIME payload
    Antik::SMTP::MailMessageBuilder builder;
    builder.setFromAddress("user@example.com");
    builder.setToAddress("recipient@example.com");
    builder.setMailSubject("Quarterly Report");
    builder.setMailMessage({"Hello,", "Please find attached the quarterly report.", "Regards"});
    builder.setHTMLMessage({"<h1>Quarterly Report</h1>", "<p>Please find attached the quarterly report.</p>"});
    builder.addFileAttachment("report.pdf", "application/pdf", "base64");

    auto payload = builder.buildPayload();

    // Post via CSMTP using XOAUTH2 Token
    Antik::SMTP::CSMTP smtp;
    smtp.setServer("smtp.gmail.com:587");
    smtp.setOAuth2Token("user@example.com", "ya29.a0AfH6SMB...");
    smtp.setFromAddress("user@example.com");
    smtp.setToAddress("recipient@example.com");
    smtp.postMail();
}
```

---

## 3. Directory Event Monitoring via `CApprise`

```cpp
#include "CApprise.hpp"
#include "CFileEventNotifier.hpp"
#include <iostream>

void watchDirectory()
{
    using namespace Antik::File;
    auto notifier = std::make_shared<CFileEventNotifier>();
    CApprise apprise(notifier, "/home/user/watch_folder");
    apprise.startWatching();

    while (apprise.stillWatching())
    {
        IApprise::Event event;
        apprise.getNextEvent(event);
        std::cout << "File Event: " << event.filePath << std::endl;
    }
}
```

---

## 4. Multi-Threaded Connection Pooling (`ConnectionPool<T>`)

```cpp
#include "CFTP.hpp"
#include "CommonUtil.hpp"
#include <iostream>

void connectionPoolExample()
{
    using namespace Antik::FTP;
    Antik::ConnectionPool<CFTP> pool([]() {
        auto ftp = std::make_unique<CFTP>();
        ftp->setServer("ftp.example.com");
        ftp->setUserAndPassword("user", "pass");
        return ftp;
    }, 4);

    // Acquire connection from pool
    auto conn = pool.acquire();
    conn->changeDir("/var/data");
    
    // Release back to pool
    pool.release(std::move(conn));
}
```
