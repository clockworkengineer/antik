#ifndef IREMOTEFILESYSTEM_HPP
#define IREMOTEFILESYSTEM_HPP

#include <string>
#include <vector>
#include "CommonAntik.hpp"

namespace Antik::Network
{
    //
    // Abstract interface for remote file system operations (OCP compliance)
    //
    class IRemoteFileSystem
    {
    public:
        virtual ~IRemoteFileSystem() = default;

        virtual void changeDir(const std::string &path) = 0;
        virtual void getCurrentWorkingDirectory(std::string &path) = 0;
        virtual void createDirectory(const std::string &path) = 0;
        virtual bool isDirectory(const std::string &path) = 0;
        virtual bool isFile(const std::string &path) = 0;
        virtual void uploadFile(const std::string &localPath, const std::string &remotePath) = 0;
        virtual void downloadFile(const std::string &remotePath, const std::string &localPath) = 0;
        virtual FileList listDirectory(const std::string &path) = 0;
    };
} // namespace Antik::Network

#endif /* IREMOTEFILESYSTEM_HPP */
