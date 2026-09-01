//
// Unit Test: UTCFTP
//
#include <gtest/gtest.h>
#include "CFTP.hpp"

using namespace Antik::FTP;

TEST(UTCFTP, DateTimeParsing)
{
    CFTP::DateTime dt("20211020123045");
    EXPECT_EQ(dt.year, 2021);
    EXPECT_EQ(dt.month, 10);
    EXPECT_EQ(dt.day, 20);
    EXPECT_EQ(dt.hour, 12);
    EXPECT_EQ(dt.minute, 30);
    EXPECT_EQ(dt.second, 45);

    std::string formatted = dt;
    EXPECT_EQ(formatted, "20211020123045");
}

TEST(UTCFTP, ConstructorSetGetServer)
{
    CFTP ftp;
    ftp.setServerAndPort("ftp.example.com", "21");
    EXPECT_EQ(ftp.getServer(), "ftp.example.com");

    ftp.setUserAndPassword("user", "pass");
    EXPECT_EQ(ftp.getUser(), "user");
    EXPECT_FALSE(ftp.isConnected());
}

TEST(UTCFTP, ConnectionFailOnNonExistentHost)
{
    CFTP ftp;
    ftp.setServerAndPort("invalid.host.local.test", "21");
    ftp.setUserAndPassword("user", "pass");

    // Connecting to non-existent server should throw or return non-zero
    EXPECT_THROW(ftp.connect(), std::runtime_error);
}
