//
// Unit Test: UTCZIP
//
#include <gtest/gtest.h>
#include "CZIP.hpp"
#include "CFile.hpp"

using namespace Antik::ZIP;
using namespace Antik::File;

TEST(UTCZIP, CreateOpenCloseEmptyZip)
{
    std::string zipPath = "/tmp/test_archive.zip";
    if (CFile::exists(zipPath))
    {
        CFile::remove(zipPath);
    }

    {
        CZIP zip(zipPath);
        zip.create();
        zip.open();
        EXPECT_TRUE(zip.isZIP64());
        EXPECT_TRUE(zip.contents().empty());
        zip.close();
    }

    EXPECT_TRUE(CFile::exists(zipPath));
    CFile::remove(zipPath);
}

TEST(UTCZIP, AddAndExtractFile)
{
    std::string zipPath = "/tmp/test_archive_add.zip";
    std::string srcPath = "/tmp/zip_src.txt";
    std::string destPath = "/tmp/zip_dest.txt";

    if (CFile::exists(zipPath)) CFile::remove(zipPath);
    if (CFile::exists(srcPath)) CFile::remove(srcPath);
    if (CFile::exists(destPath)) CFile::remove(destPath);

    std::string sampleData = "ZIP Compression Unit Test Data 123456789";
    {
        std::ofstream ofs(srcPath);
        ofs << sampleData;
    }

    {
        CZIP zip(zipPath);
        zip.create();
        zip.open();
        EXPECT_TRUE(zip.add(srcPath, "zip_src.txt"));
        zip.close();
    }

    {
        CZIP zip(zipPath);
        zip.open();
        auto details = zip.contents();
        ASSERT_EQ(details.size(), 1u);
        EXPECT_EQ(details[0].fileName, "zip_src.txt");

        EXPECT_TRUE(zip.extract("zip_src.txt", destPath));
        zip.close();
    }

    EXPECT_TRUE(CFile::exists(destPath));
    std::ifstream ifs(destPath);
    std::string extractedData((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(extractedData, sampleData);

    CFile::remove(zipPath);
    CFile::remove(srcPath);
    CFile::remove(destPath);
}
