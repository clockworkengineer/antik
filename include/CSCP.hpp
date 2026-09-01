#ifndef CSCP_HPP
#define CSCP_HPP
//
// C++ STL
//
#include <stdexcept>
#include <cstring>
//
// Antik classes
//
#include "CommonAntik.hpp"
#include "CSSHSession.hpp"
//
// Libssh
//
#include <libssh/libssh.h>
// =========
// NAMESPACE
// =========
#include "IRemoteFileSystem.hpp"

namespace Antik::SSH
{
    class CSSHSession;
    // ==========================
    // PUBLIC TYPES AND CONSTANTS
    // ==========================
    // ================
    // CLASS DEFINITION
    // ================
    class CSCP : public IOBufferManager, public Network::IRemoteFileSystem
    {
    public:
        // ==========================
        // PUBLIC TYPES AND CONSTANTS
        // ==========================
        //
        // Class exception
        //
        struct Exception : public SSHExceptionBase
        {
            Exception(CSCP &scp, const std::string functionName)
                : SSHExceptionBase("CSCP", functionName, scp.getSession().getError(), scp.getSession().getErrorCode())
            {
            }
            Exception(const std::string &errorMessage, const std::string &functionName)
                : SSHExceptionBase("CSCP", functionName, errorMessage)
            {
            }
        };
        //
        // Re-map some linux types used (possibly make these more abstract at a later date).
        //
        using FilePermissions = mode_t; // File permission (boost::filesystem status for portable way to get)
        // ============
        // CONSTRUCTORS
        // ============
        //
        // Main constructor
        //
        explicit CSCP(CSSHSession &session, int mode, const std::string &location);
        // ==========
        // DESTRUCTOR
        // ==========
        virtual ~CSCP();
        // ==============
        // PUBLIC METHODS
        // ==============
        //
        // Open/close SCP connection.
        //
        void open();
        void close();
        //
        // File/Directory remote creation.
        //
        void pushDirectory(const std::string &directoryName, FilePermissions permissions);
        void pushFile(const std::string &fileName, size_t fileSize, FilePermissions permissions);
        void pushFile64(const std::string &fileName, uint64_t fileSize, FilePermissions permissions);
        void write(const void *buffer, size_t bufferSize);
        void leaveDirectory();
        //
        // File/Directory retrieval.
        //
        int pullRequest();
        void acceptRequest();
        void denyRequest(const std::string &reason);
        std::string getRequestWarning();
        size_t requestFileSize();
        uint64_t requestFileSize64();
        std::string requestFileName();
        FilePermissions requestFilePermissions();
        int read(void *buffer, size_t bufferSize);
        //
        // Set IO buffer parameters.
        //
        std::shared_ptr<char[]> getIoBuffer();
        void setIoBufferSize(std::uint32_t ioBufferSize);
        std::uint32_t getIoBufferSize() const;
        //
        // IRemoteFileSystem overrides
        //
        void changeDir(const std::string &path) override;
        void getCurrentWorkingDirectory(std::string &path) override;
        void createDirectory(const std::string &path) override;
        bool isDirectory(const std::string &path) override;
        bool isFile(const std::string &path) override;
        void uploadFile(const std::string &localPath, const std::string &remotePath, ProgressFn progressFn = nullptr) override;
        void downloadFile(const std::string &remotePath, const std::string &localPath, ProgressFn progressFn = nullptr) override;
        FileList listDirectory(const std::string &path) override;
        //
        // Get internal libssh ssh/scp session data structure pointers.
        //
        CSSHSession &getSession() const;
        ssh_scp getSCP() const;
        // ================
        // PUBLIC VARIABLES
        // ================
    private:
        // ===========================
        // PRIVATE TYPES AND CONSTANTS
        // ===========================
        // ===========================================
        // DISABLED CONSTRUCTORS/DESTRUCTORS/OPERATORS
        // ===========================================
        CSCP() = delete;
        CSCP(const CSCP &orig) = delete;
        CSCP(const CSCP &&orig) = delete;
        CSCP &operator=(CSCP other) = delete;
        // ===============
        // PRIVATE METHODS
        // ===============
        // =================
        // PRIVATE VARIABLES
        // =================
        CSSHSession &m_session; // SSH session
        ssh_scp m_scp;          // SCP connection
        //  int m_mode {};                  // SCP mode
        std::string m_location;                      // SCP location
        std::shared_ptr<char[]> m_ioBuffer{nullptr}; // IO buffer
        std::uint32_t m_ioBufferSize{32 * 1024};     // IO buffer size
    };
} // namespace Antik::SSH
#endif /* CSCP_HPP */
