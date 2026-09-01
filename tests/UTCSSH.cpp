//
// Unit Test: UTCSSH
//
#include <gtest/gtest.h>
#include "CSSHSession.hpp"
#include "CSSHChannel.hpp"
#include "CSFTP.hpp"
#include "CSCP.hpp"

using namespace Antik::SSH;

TEST(UTCSSH, UserAuthorizationTypeEnum)
{
    EXPECT_EQ(UserAuthorizationType::None, 0x1);
    EXPECT_EQ(UserAuthorizationType::Password, 0x2);
    EXPECT_EQ(UserAuthorizationType::PublicKey, 0x4);
    EXPECT_EQ(UserAuthorizationType::Interactice, 0x8);
}

TEST(UTCSSH, CSSHSessionGetSet)
{
    CSSHSession session;
    session.setServer("ssh.example.com");

    session.setPort(2222);

    session.setUser("sshuser");

    session.setUserPassword("sshpass");
    EXPECT_FALSE(session.isConnected());
}

TEST(UTCSSH, CSSHSessionException)
{
    CSSHSession session;
    CSSHSession::Exception ex(session, "testFunc");
    EXPECT_NE(ex.getMessage().find("CSSHSession Failure: (testFunc)"), std::string::npos);
}
