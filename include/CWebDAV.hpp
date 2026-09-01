#ifndef CWEBDAV_HPP
#define CWEBDAV_HPP

#include "IRemoteFileSystem.hpp"
#include "CCurl.hpp"

namespace Antik::Network
{
    //
    // WebDAV Remote File System Provider (inherits IRemoteFileSystem)
    //
    class CWebDAV : public IRemoteFileSystem, public ServerCredentials
    {
    public:
        CWebDAV();
        virtual ~CWebDAV() = default;

        void changeDir(const std::string &path) override;
        void getCurrentWorkingDirectory(std::string &path) override;
        void createDirectory(const std::string &path) override;
        bool isDirectory(const std::string &path) override;
        bool isFile(const std::string &path) override;
        void uploadFile(const std::string &localPath, const std::string &remotePath, ProgressFn progressFn = nullptr) override;
        void downloadFile(const std::string &remotePath, const std::string &localPath, ProgressFn progressFn = nullptr) override;
        FileList listDirectory(const std::string &path) override;

    private:
        std::string m_currentDir{"/"};
        CCurl m_curl;
    };
} // namespace Antik::Network

#endif /* CWEBDAV_HPP */
