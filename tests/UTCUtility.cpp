//
// Unit Test: UTCUtility
//
#include <gtest/gtest.h>
#include "FTPUtil.hpp"
#include "SCPUtil.hpp"
#include "SFTPUtil.hpp"
#include "SSHChannelUtil.hpp"
#include "SSHSessionUtil.hpp"

using namespace Antik::SSH;

TEST(UTCUtility, IOContextDefaults)
{
    IOContext ioCtx;
    EXPECT_TRUE(ioCtx.useInternalInput());

    std::string testData = "Output data\n";
    EXPECT_NO_THROW(ioCtx.writeOutput(static_cast<void *>(const_cast<char *>(testData.c_str())), testData.size()));
    EXPECT_NO_THROW(ioCtx.writeError(static_cast<void *>(const_cast<char *>(testData.c_str())), testData.size()));
}
