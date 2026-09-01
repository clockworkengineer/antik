#ifndef IARCHIVE_HPP
#define IARCHIVE_HPP

#include <string>
#include <vector>
#include "CommonAntik.hpp"

namespace Antik::Archive
{
    //
    // Abstract Interface for Archive operations (Zip, Tar, etc.)
    //
    class IArchive
    {
    public:
        virtual ~IArchive() = default;

        virtual void open(const std::string &archivePath) = 0;
        virtual void extractTo(const std::string &destinationDirectory) = 0;
        virtual void addFile(const std::string &filePath, const std::string &archiveName) = 0;
        virtual FileList listContents() = 0;
    };
} // namespace Antik::Archive

#endif /* IARCHIVE_HPP */
