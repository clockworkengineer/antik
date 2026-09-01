//
// Unit Test: UTCCCurl
//
#include <gtest/gtest.h>
#include "CCurl.hpp"

using namespace Antik::Network;

TEST(UTCCCurl, ConstructorDestructor)
{
    EXPECT_NO_THROW({
        CCurl curl;
    });
}

TEST(UTCCCurl, SetErrorBuffer)
{
    CCurl curl;
    EXPECT_NO_THROW(curl.setErrorBuffer(256));
}

TEST(UTCCCurl, SetOptionAndGetInfo)
{
    CCurl curl;
    EXPECT_NO_THROW(curl.setOption(CURLOPT_VERBOSE, 0L));
    EXPECT_NO_THROW(curl.setOption(CURLOPT_URL, "http://localhost"));

    char *url = nullptr;
    EXPECT_NO_THROW(url = curl.getInfo<char *>(CURLINFO_EFFECTIVE_URL));
    (void)url;
}

TEST(UTCCCurl, Reset)
{
    CCurl curl;
    curl.setOption(CURLOPT_VERBOSE, 1L);
    EXPECT_NO_THROW(curl.reset());
}

TEST(UTCCCurl, StringListOperations)
{
    CCurl::StringList list = nullptr;
    EXPECT_NO_THROW(list = CCurl::stringListAppend(list, "Header: Value"));
    EXPECT_NE(list, nullptr);

    EXPECT_NO_THROW(list = CCurl::stringListAppend(list, "Header2: Value2"));
    EXPECT_NE(list, nullptr);

    EXPECT_NO_THROW(CCurl::stringListFree(list));
}

TEST(UTCCCurl, ExceptionHandling)
{
    CCurl curl;
    // Setting an invalid option should throw CCurl::Exception
    EXPECT_THROW(curl.setOption(static_cast<CURLoption>(-1), 0), CCurl::Exception);
}

TEST(UTCCCurl, GlobalCleanup)
{
    EXPECT_NO_THROW(CCurl::globalCleanup());
}

TEST(UTCCCurl, HelperMethods)
{
    CCurl curl;
    EXPECT_NO_THROW(curl.setCredentials("testuser", "testpass"));
    EXPECT_NO_THROW(curl.setSSLVerification(false, false));
    EXPECT_NO_THROW(curl.setTimeouts(5L, 30L));
}
