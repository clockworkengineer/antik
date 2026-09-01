#ifndef MAILMESSAGEBUILDER_HPP
#define MAILMESSAGEBUILDER_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace Antik::SMTP
{
    struct EmailAttachment
    {
        std::string fileName;
        std::string contentTypes;
        std::string contentTransferEncoding;
        std::vector<std::string> encodedContents{};
    };

    class MailMessageBuilder
    {
    public:
        MailMessageBuilder();
        virtual ~MailMessageBuilder() = default;

        void setFromAddress(const std::string &addressFrom);
        void setToAddress(const std::string &addressTo);
        void setCCAddress(const std::string &addressCC);
        void setMailSubject(const std::string &subject);
        void setMailMessage(const std::vector<std::string> &message);
        void addFileAttachment(const std::string &fileName, const std::string &contentType, const std::string &contentTransferEncoding);

        std::vector<std::string> buildPayload();

    private:
        void encodeAttachment(EmailAttachment &attachment);
        void buildAttachments(std::vector<std::string> &payload);
        static std::string currentDateAndTime();

        std::string m_addressFrom;
        std::string m_addressTo;
        std::string m_addressCC;
        std::string m_mailSubject;
        std::vector<std::string> m_mailMessage;
        std::vector<EmailAttachment> m_attachedFiles;
    };
} // namespace Antik::SMTP

#endif /* MAILMESSAGEBUILDER_HPP */
