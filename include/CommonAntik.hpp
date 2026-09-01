#ifndef COMMON_ANTIK_HPP
#define COMMON_ANTIK_HPP
//
// C++ STL
//
#include <string>
#include <vector>
#include <cstdint>
namespace Antik
{
    //
    // Container for list of file paths
    //
    using FileList = std::vector<std::string>;
    //
    // Server path separator
    //
    const char kServerPathSep{'/'};

    //
    // Shared Server Credentials class
    //
    class ServerCredentials
    {
    public:
        virtual ~ServerCredentials() = default;
        void setServer(const std::string &server) { m_serverName = server; }
        void setPort(const std::string &port) { m_serverPort = port; }
        void setUser(const std::string &user) { m_userName = user; }
        void setUserPassword(const std::string &password) { m_userPassword = password; }
        void setUserAndPassword(const std::string &userName, const std::string &userPassword)
        {
            m_userName = userName;
            m_userPassword = userPassword;
        }
        std::string getServer(void) const { return m_serverName; }
        std::string getPort(void) const { return m_serverPort; }
        std::string getUser(void) const { return m_userName; }

    protected:
        std::string m_serverName;
        std::string m_serverPort{"21"};
        std::string m_userName;
        std::string m_userPassword;
    };

    //
    // Shared I/O Buffer Manager class
    //
    class IOBufferManager
    {
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

    //
    // Generic Exception Wrapper
    //
    template <typename ExceptionType, typename Func>
    auto wrapException(Func &&func)
    {
        try
        {
            return func();
        }
        catch (const std::exception &e)
        {
            throw ExceptionType(e.what());
        }
    }
} // namespace Antik

namespace Antik::SSH
{
    //
    // Base class for SSH module exceptions
    //
    class SSHExceptionBase
    {
    public:
        SSHExceptionBase(const std::string &className, const std::string &functionName, const std::string &errorMessage, int errorCode = 0)
            : m_className{className}, m_functionName{functionName}, m_errorMessage{errorMessage}, m_errorCode{errorCode}
        {
        }

        virtual ~SSHExceptionBase() = default;

        int getCode() const
        {
            return m_errorCode;
        }

        std::string getMessage() const
        {
            return m_className + " Failure: (" + m_functionName + ") [" + m_errorMessage + "]";
        }

    protected:
        std::string m_className;
        std::string m_functionName;
        std::string m_errorMessage;
        int m_errorCode{0};
    };
} // namespace Antik::SSH
#endif /* COMMON_HPP */
