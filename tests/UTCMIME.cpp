//
// Unit Test: UTCMIME
//
#include <gtest/gtest.h>
#include "CMIME.hpp"

using namespace Antik::File;

TEST(UTCMIME, GetFileMIMEType)
{
    EXPECT_EQ(CMIME::getFileMIMEType("test.pdf"), "application/pdf");
    EXPECT_EQ(CMIME::getFileMIMEType("image.png"), "image/png");
    EXPECT_EQ(CMIME::getFileMIMEType("photo.jpg"), "image/jpeg");
    EXPECT_EQ(CMIME::getFileMIMEType("document.txt"), "text/plain");
    EXPECT_EQ(CMIME::getFileMIMEType("page.html"), "text/html");
    EXPECT_EQ(CMIME::getFileMIMEType("unknown_file_ext.xyz123"), "application/unknown");
}

TEST(UTCMIME, ConvertMIMEStringToASCIIPlain)
{
    std::string plain = "Hello World\r\n";
    EXPECT_EQ(CMIME::convertMIMEStringToASCII(plain), "Hello World");
}

TEST(UTCMIME, ConvertMIMEStringToASCIIBase64)
{
    std::string mimeB64 = "=?UTF-8?B?SGVsbG8gV29ybGQ=?=\r\n";
    EXPECT_STREQ(CMIME::convertMIMEStringToASCII(mimeB64).c_str(), "Hello World");
}

TEST(UTCMIME, ConvertMIMEStringToASCIIQuotedPrintable)
{
    std::string mimeQP = "=?UTF-8?Q?Hello=20World?=\r\n";
    EXPECT_EQ(CMIME::convertMIMEStringToASCII(mimeQP), "Hello World");
}
