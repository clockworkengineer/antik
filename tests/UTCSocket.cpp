//
// Unit Test: UTCSocket
//
#include <gtest/gtest.h>
#include "CSocket.hpp"

using namespace Antik::Network;

TEST(UTCSocket, ExceptionType)
{
    CSocket::Exception ex("Connection error");
    EXPECT_NE(std::string(ex.what()).find("CSocket Failure: Connection error"), std::string::npos);
}

TEST(UTCSocket, TLSVersionEnum)
{
    EXPECT_EQ(CSocket::v1_0, 0);
    EXPECT_EQ(CSocket::v1_1, 1);
    EXPECT_EQ(CSocket::v1_2, 2);
}

TEST(UTCSocket, ConstructorDestructor)
{
    EXPECT_NO_THROW({
        CSocket socket;
    });
}
