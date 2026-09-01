#ifndef ISSHSESSION_HPP
#define ISSHSESSION_HPP

#include <string>

namespace Antik::SSH
{
    //
    // Abstract interface for SSH sessions (DIP compliance)
    //
    class ISSHSession
    {
    public:
        virtual ~ISSHSession() = default;

        virtual void setServer(const std::string &serverURL) = 0;
        virtual void setUserAndPassword(const std::string &userName, const std::string &userPassword) = 0;
        virtual void connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
        virtual std::string getError() const = 0;
        virtual int getErrorCode() const = 0;
    };
} // namespace Antik::SSH

#endif /* ISSHSESSION_HPP */
