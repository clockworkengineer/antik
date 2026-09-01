#ifndef TESTHELPERS_HPP
#define TESTHELPERS_HPP

#include <string>
#include <fstream>
#include <filesystem>

namespace Antik::Test
{
    //
    // RAII Helper for Temporary Test Files
    //
    class TempTestFile
    {
    public:
        explicit TempTestFile(const std::string &filepath, const std::string &content = "Test File Content")
            : m_filepath(filepath)
        {
            std::ofstream ofs(m_filepath, std::ios::binary);
            ofs << content;
        }

        ~TempTestFile()
        {
            std::error_code ec;
            std::filesystem::remove(m_filepath, ec);
        }

        std::string getPath() const
        {
            return m_filepath;
        }

        bool exists() const
        {
            return std::filesystem::exists(m_filepath);
        }

    private:
        std::string m_filepath;
    };

    //
    // RAII Helper for Temporary Test Directories
    //
    class TempTestDir
    {
    public:
        explicit TempTestDir(const std::string &dirpath)
            : m_dirpath(dirpath)
        {
            std::error_code ec;
            std::filesystem::create_directories(m_dirpath, ec);
        }

        ~TempTestDir()
        {
            std::error_code ec;
            std::filesystem::remove_all(m_dirpath, ec);
        }

        std::string getPath() const
        {
            return m_dirpath;
        }

        bool exists() const
        {
            return std::filesystem::exists(m_dirpath);
        }

    private:
        std::string m_dirpath;
    };
} // namespace Antik::Test

#endif /* TESTHELPERS_HPP */
