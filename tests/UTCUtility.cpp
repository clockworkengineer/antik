//
// Unit Test: UTCUtility (StringUtil, CProgramOptions, IOContext)
//
#include <gtest/gtest.h>
#include "CommonUtil.hpp"
#include "CProgramOptions.hpp"
#include "FTPUtil.hpp"
#include "SCPUtil.hpp"
#include "SFTPUtil.hpp"
#include "SSHChannelUtil.hpp"
#include "SSHSessionUtil.hpp"
#include <fstream>
#include <cstdio>

using namespace Antik;
using namespace Antik::Util;
using namespace Antik::SSH;

TEST(UTCUtility, IOContextDefaults)
{
    IOContext ioCtx;
    EXPECT_TRUE(ioCtx.useInternalInput());

    std::string testData = "Output data\n";
    EXPECT_NO_THROW(ioCtx.writeOutput(static_cast<void *>(const_cast<char *>(testData.c_str())), testData.size()));
    EXPECT_NO_THROW(ioCtx.writeError(static_cast<void *>(const_cast<char *>(testData.c_str())), testData.size()));
}

TEST(UTCUtility, StringUtilTrimAndStripping)
{
    EXPECT_EQ(StringUtil::trim("  hello world  "), "hello world");
    EXPECT_EQ(StringUtil::trim("\t\n test \r\n"), "test");
    EXPECT_EQ(StringUtil::trim("no_spaces"), "no_spaces");
    EXPECT_EQ(StringUtil::trim("   "), "");

    EXPECT_EQ(StringUtil::stripQuotes("\"quoted string\""), "quoted string");
    EXPECT_EQ(StringUtil::stripQuotes("unquoted"), "unquoted");

    EXPECT_EQ(StringUtil::stripOuterParens("(nested parens)"), "nested parens");
    EXPECT_EQ(StringUtil::stripOuterParens("no parens"), "no parens");
}

TEST(UTCUtility, StringUtilComparisons)
{
    EXPECT_TRUE(StringUtil::equalsIgnoreCase("Hello", "hello"));
    EXPECT_TRUE(StringUtil::equalsIgnoreCase("ANTIK", "antik"));
    EXPECT_FALSE(StringUtil::equalsIgnoreCase("Hello", "World"));

    EXPECT_TRUE(StringUtil::startsWithIgnoreCase("IMAP4rev1", "imap"));
    EXPECT_TRUE(StringUtil::startsWithIgnoreCase("SMTP_SERVER", "smtp"));
    EXPECT_FALSE(StringUtil::startsWithIgnoreCase("POP3_SERVER", "smtp"));
}

TEST(UTCUtility, CProgramOptionsParsing)
{
    CProgramOptions opts("Test Options");
    opts.addOption("s", "server", "Server URL", true, "localhost");
    opts.addOption("p", "port", "Server port", true, "143");
    opts.addOption("v", "verbose", "Verbose mode");

    const char *argv[] = {"test_app", "-s", "imap.example.com", "-p", "993", "-v"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_TRUE(opts.parse(argc, const_cast<char**>(argv)));
    EXPECT_TRUE(opts.count("server"));
    EXPECT_EQ(opts.get("server"), "imap.example.com");
    EXPECT_TRUE(opts.count("port"));
    EXPECT_EQ(opts.getInt("port"), 993);
    EXPECT_TRUE(opts.count("verbose"));
}

TEST(UTCUtility, CProgramOptionsDefaultsAndConfigFile)
{
    CProgramOptions opts("Config Test");
    opts.addOption("u", "user", "Account username", true, "default_user");
    opts.addOption("w", "timeout", "Timeout seconds", true, "30");

    EXPECT_EQ(opts.get("user"), "default_user");
    EXPECT_EQ(opts.getInt("timeout"), 30);

    // Create temporary config file
    std::string configPath = "/tmp/antik_test_config.ini";
    std::ofstream cfg(configPath);
    cfg << "# Test Configuration\n";
    cfg << "user = john_doe\n";
    cfg << "timeout = 60\n";
    cfg.close();

    EXPECT_TRUE(opts.loadConfigFile(configPath));
    EXPECT_EQ(opts.get("user"), "john_doe");
    EXPECT_EQ(opts.getInt("timeout"), 60);

    std::remove(configPath.c_str());
}
