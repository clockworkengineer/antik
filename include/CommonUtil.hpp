#ifndef COMMONUTIL_HPP
#define COMMONUTIL_HPP
// =============
// INCLUDE FILES
// =============
//
// C++ STL
//
#include <filesystem>
//
// Antik classes
//
#include "CPath.hpp"
#include "CommonAntik.hpp"
//
// Boost file system, string
//
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
namespace Antik
{
    //
    // File transfer complete function
    //
    typedef std::function<void(const std::string &)> FileCompletionFn;
    //
    // Remote file recursive list feedback function
    //
    typedef std::function<void(const std::string &)> FileFeedBackFn;
    //
    // Map files from to/from local/remote directories
    //
    class FileMapper
    {
    public:
        explicit FileMapper(const std::string &localDirectory, const std::string &remoteDirectory) : m_localDirectory{localDirectory}, m_remoteDirectory{remoteDirectory}
        {
            if (m_localDirectory.back() == kServerPathSep)
                m_localDirectory.pop_back();
            if (m_remoteDirectory.back() == kServerPathSep)
                m_remoteDirectory.pop_back();
        }
        std::string toLocal(const std::string &filePath)
        {
            File::CPath localPath{m_localDirectory + kServerPathSep + filePath.substr(m_remoteDirectory.size())};
            localPath.normalize();
            return (localPath.toString());
        }
        std::string toRemote(const std::string &filePath)
        {
            File::CPath remotePath{m_remoteDirectory + kServerPathSep + filePath.substr(m_localDirectory.size())};
            remotePath.normalize();
            return (remotePath.toString());
        }
        std::string getRemoteDirectory() const
        {
            return m_remoteDirectory;
        }
        std::string getLocalDirectory() const
        {
            return m_localDirectory;
        }

    private:
        std::string m_localDirectory;
        std::string m_remoteDirectory;
    };
    //
    // Recursively parse a local directory and produce a list of files. (static for moment)
    //
    static inline void listLocalRecursive(const std::string &localDirectory, FileList &fileList, FileFeedBackFn localFileFeedbackFn = nullptr)
    {
        for (auto directoryEntry : std::filesystem::recursive_directory_iterator{localDirectory})
        {
            fileList.push_back(directoryEntry.path().string());
            if (localFileFeedbackFn)
            {
                localFileFeedbackFn(directoryEntry.path().string());
            }
        }
    }

    //
    // Base64 encoding/decoding utility
    //
    class Base64
    {
    public:
        static void encode(const std::string &decoding, std::string &encoding, std::uint32_t numberOfBytes)
        {
            static const char kCB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            if (numberOfBytes == 0) return;
            encoding.clear();
            std::uint32_t trailing = (numberOfBytes % 3);
            std::uint32_t count = numberOfBytes / 3;
            int byteIndex = 0;
            std::uint8_t byte1, byte2, byte3;
            while (count--)
            {
                byte1 = decoding[byteIndex++];
                byte2 = decoding[byteIndex++];
                byte3 = decoding[byteIndex++];
                encoding.append(1, kCB64[(byte1 & 0xfc) >> 2]);
                encoding.append(1, kCB64[((byte1 & 0x03) << 4) + ((byte2 & 0xf0) >> 4)]);
                encoding.append(1, kCB64[((byte2 & 0x0f) << 2) + ((byte3 & 0xc0) >> 6)]);
                encoding.append(1, kCB64[byte3 & 0x3f]);
            }
            if (trailing == 1)
            {
                byte1 = decoding[byteIndex++];
                encoding.append(1, kCB64[(byte1 & 0xfc) >> 2]);
                encoding.append(1, kCB64[(byte1 & 0x03) << 4]);
                encoding.append(1, '=');
                encoding.append(1, '=');
            }
            else if (trailing == 2)
            {
                byte1 = decoding[byteIndex++];
                byte2 = decoding[byteIndex++];
                encoding.append(1, kCB64[(byte1 & 0xfc) >> 2]);
                encoding.append(1, kCB64[((byte1 & 0x03) << 4) + ((byte2 & 0xf0) >> 4)]);
                encoding.append(1, kCB64[(byte2 & 0x0f) << 2]);
                encoding.append(1, '=');
            }
        }

        static void decode(const std::string &encoding, std::string &decoding, std::uint32_t numberOfBytes)
        {
            auto decodeChar = [](char ch) -> int {
                if (ch >= 'A' && ch <= 'Z') return ch - 'A';
                if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
                if (ch >= '0' && ch <= '9') return ch - '0' + 52;
                if (ch == '+') return 62;
                if (ch == '/') return 63;
                return 0;
            };

            if ((numberOfBytes == 0) || (numberOfBytes % 4)) return;
            decoding.clear();
            std::uint32_t count = numberOfBytes / 4;
            int byteIndex = 0;
            std::uint8_t byte1, byte2, byte3, byte4;
            while (count--)
            {
                byte1 = encoding[byteIndex++];
                byte2 = encoding[byteIndex++];
                byte3 = encoding[byteIndex++];
                byte4 = encoding[byteIndex++];
                byte1 = decodeChar(byte1);
                byte2 = decodeChar(byte2);
                if (byte3 == '=')
                {
                    byte3 = 0;
                    byte4 = 0;
                }
                else if (byte4 == '=')
                {
                    byte3 = decodeChar(byte3);
                    byte4 = 0;
                }
                else
                {
                    byte3 = decodeChar(byte3);
                    byte4 = decodeChar(byte4);
                }
                decoding.append(1, (byte1 << 2) | (byte2 >> 4));
                if (byte3) decoding.append(1, ((byte2 & 0x0f) << 4) | (byte3 >> 2));
                if (byte4) decoding.append(1, ((byte3 & 0x03) << 6) | byte4);
            }
        }
    };
} // namespace Antik
#endif /* COMMONUTIL_HPP */
