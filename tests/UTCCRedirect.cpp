//
// Unit Test: UTCCRedirect
//
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "CRedirect.hpp"
#include "CFile.hpp"

using namespace Antik::Util;
using namespace Antik::File;

TEST(UTCCRedirect, StreamRedirectConstructor)
{
    std::string testFile{"/tmp/test_redirect_1.txt"};

    {
        std::ostringstream testStream;
        CRedirect redirect(testStream);
        redirect.change(testFile, std::ios_base::out);
        testStream << "Hello Redirect";
    }

    EXPECT_TRUE(CFile::exists(testFile));
    std::ifstream ifs(testFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("Hello Redirect"), std::string::npos);

    CFile::remove(testFile);
}

TEST(UTCCRedirect, StreamRedirectWithFilenameConstructor)
{
    std::string testFile{"/tmp/test_redirect_2.txt"};

    {
        std::ostringstream testStream;
        CRedirect redirect(testStream, testFile);
        testStream << "Stream Redirect Content";
    }

    EXPECT_TRUE(CFile::exists(testFile));
    std::ifstream ifs(testFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("Stream Redirect Content"), std::string::npos);

    CFile::remove(testFile);
}

TEST(UTCCRedirect, RestoreExplicitCall)
{
    std::string testFile{"/tmp/test_redirect_3.txt"};
    std::ostringstream testStream;

    {
        CRedirect redirect(testStream, testFile);
        testStream << "Before Restore";
        redirect.restore();
        testStream << "After Restore";
    }

    EXPECT_TRUE(CFile::exists(testFile));
    std::ifstream ifs(testFile);
    std::string fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(fileContent.find("Before Restore"), std::string::npos);
    EXPECT_EQ(fileContent.find("After Restore"), std::string::npos);

    EXPECT_NE(testStream.str().find("After Restore"), std::string::npos);

    CFile::remove(testFile);
}
