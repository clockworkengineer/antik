//
// Unit Test: UTCZPIO
//
#include <gtest/gtest.h>
#include "CZIPIO.hpp"
#include "CFile.hpp"

using namespace Antik::ZIP;
using namespace Antik::File;

TEST(UTCZPIO, StructSizesAndConstants)
{
    EXPECT_EQ(kZIPCompressionStore, 0);
    EXPECT_EQ(kZIPCompressionDeflate, 8);
    EXPECT_EQ(kZIPVersion10, 0x0a);
    EXPECT_EQ(kZIPVersion20, 0x14);
}

TEST(UTCZPIO, LocalFileHeaderDefaults)
{
    CZIPIO::LocalFileHeader lfh;
    EXPECT_EQ(lfh.signature, 0x04034b50u);
    EXPECT_EQ(lfh.size, 30u);
    EXPECT_EQ(lfh.crc32, 0u);
}

TEST(UTCZPIO, CentralDirectoryFileHeaderDefaults)
{
    CZIPIO::CentralDirectoryFileHeader cdfh;
    EXPECT_EQ(cdfh.signature, 0x02014b50u);
    EXPECT_EQ(cdfh.size, 46u);
}

TEST(UTCZPIO, EndOfCentralDirectoryRecordDefaults)
{
    CZIPIO::EOCentralDirectoryRecord eocd;
    EXPECT_EQ(eocd.signature, 0x06054b50u);
    EXPECT_EQ(eocd.size, 22u);
}
