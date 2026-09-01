#include "MailMessageBuilder.hpp"
#include "CommonUtil.hpp"
#include <fstream>
#include <ctime>

namespace Antik::SMTP
{
    static const char *kMimeBoundary{"----=_Part_0_123456789.987654321"};
    static const char *kEOL{"\r\n"};
    static const std::size_t kBase64EncodeBufferSize{54};

    MailMessageBuilder::MailMessageBuilder() = default;

    void MailMessageBuilder::setFromAddress(const std::string &addressFrom)
    {
        m_addressFrom = addressFrom;
    }

    void MailMessageBuilder::setToAddress(const std::string &addressTo)
    {
        m_addressTo = addressTo;
    }

    void MailMessageBuilder::setCCAddress(const std::string &addressCC)
    {
        m_addressCC = addressCC;
    }

    void MailMessageBuilder::setMailSubject(const std::string &subject)
    {
        m_mailSubject = subject;
    }

    void MailMessageBuilder::setMailMessage(const std::vector<std::string> &message)
    {
        m_mailMessage = message;
    }

    void MailMessageBuilder::setHTMLMessage(const std::vector<std::string> &htmlMessage)
    {
        m_htmlMessage = htmlMessage;
    }

    void MailMessageBuilder::addFileAttachment(const std::string &fileName, const std::string &contentType, const std::string &contentTransferEncoding)
    {
        m_attachedFiles.push_back({fileName, contentType, contentTransferEncoding});
    }

    std::string MailMessageBuilder::currentDateAndTime()
    {
        std::time_t timeNow = std::time(nullptr);
        struct tm tmBuffer;
        char dateBuffer[128];
        struct tm *timeInfo = localtime_r(&timeNow, &tmBuffer);
        std::strftime(dateBuffer, sizeof(dateBuffer), "%a, %d %b %Y %H:%M:%S %z", timeInfo);
        return std::string(dateBuffer);
    }

    void MailMessageBuilder::encodeAttachment(EmailAttachment &attachment)
    {
        std::string line;
        if (attachment.contentTransferEncoding != "base64")
        {
            std::ifstream attachmentFile(attachment.fileName);
            while (std::getline(attachmentFile, line))
            {
                if (!line.empty() && line.back() == '\n') line.pop_back();
                if (!line.empty() && line.back() == '\r') line.pop_back();
                attachment.encodedContents.push_back(line + kEOL);
            }
        }
        else
        {
            std::ifstream ifs{attachment.fileName, std::ios::binary};
            std::string buffer(kBase64EncodeBufferSize, ' ');
            while (ifs.good())
            {
                ifs.read(&buffer[0], kBase64EncodeBufferSize);
                if (ifs.gcount() > 0)
                {
                    Base64::encode(buffer, line, ifs.gcount());
                    attachment.encodedContents.push_back(line + kEOL);
                    line.clear();
                }
            }
        }
    }

    void MailMessageBuilder::buildAttachments(std::vector<std::string> &payload)
    {
        for (auto &attachment : m_attachedFiles)
        {
            std::string baseFileName{attachment.fileName.substr(attachment.fileName.find_last_of(R"(/\)") + 1)};
            encodeAttachment(attachment);
            payload.emplace_back(std::string("--") + kMimeBoundary + kEOL);
            payload.emplace_back("Content-Type: " + attachment.contentTypes + ";" + kEOL);
            payload.emplace_back("Content-transfer-encoding: " + attachment.contentTransferEncoding + kEOL);
            payload.emplace_back(std::string("Content-Disposition: attachment;") + kEOL);
            payload.emplace_back(R"(     filename=")" + baseFileName + R"(")" + kEOL);
            payload.emplace_back(kEOL);
            for (const auto &str : attachment.encodedContents)
            {
                payload.emplace_back(str);
            }
            payload.emplace_back(kEOL);
        }
    }

    std::vector<std::string> MailMessageBuilder::buildPayload()
    {
        std::vector<std::string> payload;
        bool hasAttachments{!m_attachedFiles.empty()};
        payload.emplace_back("Date: " + currentDateAndTime() + kEOL);
        payload.emplace_back("To: " + m_addressTo + kEOL);
        payload.emplace_back("From: " + m_addressFrom + kEOL);
        if (!m_addressCC.empty())
        {
            payload.emplace_back("cc: " + m_addressCC + kEOL);
        }
        payload.emplace_back("Subject: " + m_mailSubject + kEOL);
        payload.emplace_back(std::string("MIME-Version: 1.0") + kEOL);
        if (!hasAttachments)
        {
            payload.emplace_back(std::string("Content-Type: text/plain; charset=UTF-8") + kEOL);
            payload.emplace_back(std::string("Content-Transfer-Encoding: 7bit") + kEOL);
        }
        else
        {
            payload.emplace_back(std::string("Content-Type: multipart/mixed;") + kEOL);
            payload.emplace_back(std::string(R"(     boundary=")") + kMimeBoundary + R"(")" + kEOL);
        }
        payload.emplace_back(kEOL);
        if (hasAttachments)
        {
            payload.emplace_back(std::string("--") + kMimeBoundary + kEOL);
            payload.emplace_back(std::string("Content-Type: text/plain") + kEOL);
            payload.emplace_back(std::string("Content-Transfer-Encoding: 7bit") + kEOL);
            payload.emplace_back(kEOL);
        }
        for (const auto &str : m_mailMessage)
        {
            payload.emplace_back(str + kEOL);
        }
        if (hasAttachments)
        {
            payload.emplace_back(kEOL);
            buildAttachments(payload);
            payload.emplace_back(std::string("--") + kMimeBoundary + "--" + kEOL);
        }
        return payload;
    }
} // namespace Antik::SMTP
