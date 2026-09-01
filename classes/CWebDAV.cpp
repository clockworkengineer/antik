#include "CWebDAV.hpp"

namespace Antik::Network
{
    CWebDAV::CWebDAV() = default;

    void CWebDAV::changeDir(const std::string &path)
    {
        m_currentDir = path;
    }

    void CWebDAV::getCurrentWorkingDirectory(std::string &path)
    {
        path = m_currentDir;
    }

    void CWebDAV::createDirectory(const std::string &path)
    {
        (void)path;
    }

    bool CWebDAV::isDirectory(const std::string &path)
    {
        (void)path;
        return false;
    }

    bool CWebDAV::isFile(const std::string &path)
    {
        (void)path;
        return true;
    }

    void CWebDAV::uploadFile(const std::string &localPath, const std::string &remotePath, ProgressFn progressFn)
    {
        (void)localPath;
        (void)remotePath;
        (void)progressFn;
    }

    void CWebDAV::downloadFile(const std::string &remotePath, const std::string &localPath, ProgressFn progressFn)
    {
        (void)remotePath;
        (void)localPath;
        (void)progressFn;
    }

    FileList CWebDAV::listDirectory(const std::string &path)
    {
        (void)path;
        return FileList{};
    }
} // namespace Antik::Network
