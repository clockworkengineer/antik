//
// Unit Test: UTCIMAPBodyStruct
//
#include <gtest/gtest.h>
#include "CIMAPBodyStruct.hpp"

using namespace Antik::IMAP;

TEST(UTCIMAPBodyStruct, SimpleSinglePartBodyStruct)
{
    std::string bodyStructStr = "((\"TEXT\" \"PLAIN\" (\"CHARSET\" \"US-ASCII\") NIL NIL \"7BIT\" 1152 23 NIL NIL NIL NIL) \"MIXED\")";
    std::unique_ptr<CIMAPBodyStruct::BodyNode> root = std::make_unique<CIMAPBodyStruct::BodyNode>();

    EXPECT_NO_THROW(CIMAPBodyStruct::consructBodyStructTree(root, bodyStructStr));
    ASSERT_NE(root, nullptr);
    EXPECT_FALSE(root->bodyParts.empty());

    if (!root->bodyParts.empty())
    {
        const auto &part = root->bodyParts[0];
        ASSERT_NE(part.parsedPart, nullptr);
        EXPECT_EQ(part.parsedPart->type, "TEXT");
        EXPECT_EQ(part.parsedPart->subtype, "PLAIN");
        EXPECT_EQ(part.parsedPart->encoding, "7BIT");
    }
}

TEST(UTCIMAPBodyStruct, MultiPartBodyStruct)
{
    std::string multiPartStr = "((\"TEXT\" \"PLAIN\" (\"CHARSET\" \"US-ASCII\") NIL NIL \"7BIT\" 1152 23)(\"TEXT\" \"HTML\" (\"CHARSET\" \"US-ASCII\") NIL NIL \"7BIT\" 2341 45) \"ALTERNATIVE\")";
    std::unique_ptr<CIMAPBodyStruct::BodyNode> root = std::make_unique<CIMAPBodyStruct::BodyNode>();

    EXPECT_NO_THROW(CIMAPBodyStruct::consructBodyStructTree(root, multiPartStr));
    ASSERT_NE(root, nullptr);

    int count = 0;
    CIMAPBodyStruct::BodyPartFn countFn = [&count](std::unique_ptr<CIMAPBodyStruct::BodyNode> &, CIMAPBodyStruct::BodyPart &part, std::shared_ptr<void> &) {
        count++;
        EXPECT_NE(part.parsedPart, nullptr);
    };

    std::shared_ptr<void> data = nullptr;
    CIMAPBodyStruct::walkBodyStructTree(root, countFn, data);
    EXPECT_EQ(count, 2);
}

TEST(UTCIMAPBodyStruct, AttachmentExtraction)
{
    std::string bodyStructWithAttachment = "((\"APPLICATION\" \"PDF\" (\"NAME\" \"report.pdf\") NIL NIL \"BASE64\" 12345 NIL (\"ATTACHMENT\" (\"FILENAME\" \"report.pdf\" \"SIZE\" \"12345\" \"CREATION-DATE\" \"Wed, 20 Oct 2021 12:00:00 +0000\")) NIL NIL) \"MIXED\")";
    std::unique_ptr<CIMAPBodyStruct::BodyNode> root = std::make_unique<CIMAPBodyStruct::BodyNode>();

    CIMAPBodyStruct::consructBodyStructTree(root, bodyStructWithAttachment);

    CIMAPBodyStruct::AttachmentData attachmentData;
    std::shared_ptr<void> walkData(&attachmentData, [](void *){});

    CIMAPBodyStruct::walkBodyStructTree(root, CIMAPBodyStruct::attachmentFn, walkData);

    ASSERT_EQ(attachmentData.attachmentsList.size(), 1u);
    EXPECT_EQ(attachmentData.attachmentsList[0].fileName, "report.pdf");
    EXPECT_EQ(attachmentData.attachmentsList[0].size, "12345");
}

TEST(UTCIMAPBodyStruct, InvalidBodyStructParseError)
{
    std::string invalidStr = "((\"TEXT\" \"PLAIN\" NIL NIL NIL NIL NIL NIL INVALID_UNQUOTED_TOKEN NIL NIL))";
    std::unique_ptr<CIMAPBodyStruct::BodyNode> root = std::make_unique<CIMAPBodyStruct::BodyNode>();

    EXPECT_THROW(CIMAPBodyStruct::consructBodyStructTree(root, invalidStr), std::exception);
}
