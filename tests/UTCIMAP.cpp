//
// Unit Test: UTCIMAP
//
#include <gtest/gtest.h>
#include "CIMAP.hpp"

using namespace Antik::IMAP;

TEST(UTCIMAP, ConstantsAndCommands)
{
    EXPECT_STREQ(kEOL, "\r\n");
    EXPECT_STREQ(kLOGIN, "LOGIN");
    EXPECT_STREQ(kSELECT, "SELECT");
    EXPECT_STREQ(kFETCH, "FETCH");
    EXPECT_STREQ(kOK, "OK");
    EXPECT_STREQ(kBAD, "BAD");
}

TEST(UTCIMAP, ConstructorGetSet)
{
    CIMAP imap;
    imap.setServer("imap.example.com");
    EXPECT_EQ(imap.getServer(), "imap.example.com");

    imap.setUserAndPassword("user@example.com", "secretpass");
    EXPECT_EQ(imap.getUser(), "user@example.com");

    imap.setIOBufferSize(16384);
    EXPECT_FALSE(imap.getConnectedStatus());
}

TEST(UTCIMAP, ExceptionType)
{
    CIMAP::Exception ex("Test Error");
    EXPECT_NE(std::string(ex.what()).find("CIMAP Failure: Test Error"), std::string::npos);
}
