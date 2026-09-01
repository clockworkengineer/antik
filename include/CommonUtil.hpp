#ifndef COMMONUTIL_HPP
#define COMMONUTIL_HPP
// =============
// INCLUDE FILES
// =============
//
// C++ STL
//
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <cctype>
#include <sstream>
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
            File::CPath::stripTrailingSeparator(m_localDirectory);
            File::CPath::stripTrailingSeparator(m_remoteDirectory);
        }
        std::string toLocal(const std::string &filePath) const
        {
            File::CPath localPath{m_localDirectory + kServerPathSep + filePath.substr(m_remoteDirectory.size())};
            localPath.normalize();
            return (localPath.toString());
        }
        std::string toRemote(const std::string &filePath) const
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
    // Synchronization & transfer engine helper
    //
    template <typename ServerType>
    class DirectorySyncEngine
    {
    public:
        static void processFileList(ServerType &server, const FileMapper &mapper, const FileList &fileList,
                                    std::function<void(ServerType &, const std::string &, const std::string &)> transferFn)
        {
            for (const auto &file : fileList)
            {
                std::string target = mapper.toRemote(file);
                transferFn(server, file, target);
            }
        }
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
    // Thread-safe event queue wrapper
    //
    template <typename T>
    class ThreadQueue
    {
    public:
        void push(T item)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(item));
            m_cv.notify_one();
        }

        bool pop(T &item, const std::function<bool()> &stopCondition = nullptr)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]() {
                return !m_queue.empty() || (stopCondition && stopCondition());
            });
            if (!m_queue.empty())
            {
                item = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }
            return false;
        }

        void notifyAll()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cv.notify_all();
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::queue<T> emptyQueue;
            std::swap(m_queue, emptyQueue);
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        std::queue<T> m_queue;
    };

    //
    // Common string manipulation and tokenization helpers
    //
    class StringUtil
    {
    public:
        static std::string trim(const std::string &str)
        {
            auto start = str.find_first_not_of(" \t\n\r\f\v");
            if (start == std::string::npos) return "";
            auto end = str.find_last_not_of(" \t\n\r\f\v");
            return str.substr(start, end - start + 1);
        }

        static std::string stripQuotes(const std::string &str)
        {
            if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
            {
                return str.substr(1, str.size() - 2);
            }
            return str;
        }

        static std::string stripOuterParens(const std::string &str)
        {
            if (str.size() >= 2 && str.front() == '(' && str.back() == ')')
            {
                return str.substr(1, str.size() - 2);
            }
            return str;
        }

        static bool equalsIgnoreCase(const std::string &a, const std::string &b)
        {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
                return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
            });
        }

        static bool startsWithIgnoreCase(const std::string &str, const std::string &prefix)
        {
            if (str.size() < prefix.size()) return false;
            return std::equal(prefix.begin(), prefix.end(), str.begin(), [](char c1, char c2) {
                return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
            });
        }

        static std::vector<std::string> split(const std::string &str, char delimiter)
        {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(str);
            while (std::getline(tokenStream, token, delimiter))
            {
                if (!token.empty())
                {
                    tokens.push_back(token);
                }
            }
            return tokens;
        }
    };

    //
    // Little-endian binary I/O serialization helper
    //
    class EndianIO
    {
    public:
        template <typename T>
        static void writeLittleEndian(T val, std::vector<std::uint8_t> &buffer)
        {
            for (std::size_t i = 0; i < sizeof(T); ++i)
            {
                buffer.push_back(static_cast<std::uint8_t>((val >> (i * 8)) & 0xFF));
            }
        }

        template <typename T>
        static const std::uint8_t *readLittleEndian(T &val, const std::uint8_t *buffer)
        {
            val = 0;
            for (std::size_t i = 0; i < sizeof(T); ++i)
            {
                val |= (static_cast<T>(buffer[i]) << (i * 8));
            }
            return buffer + sizeof(T);
        }
    };

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
