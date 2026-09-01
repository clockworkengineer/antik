//
// Unit Test: UTCFileEventNotifier
//
#include <gtest/gtest.h>
#include "CFileEventNotifier.hpp"
#include "CFile.hpp"
#include <thread>
#include <chrono>

using namespace Antik::File;

TEST(UTCFileEventNotifier, ConstructorDestructor)
{
    EXPECT_NO_THROW({
        CFileEventNotifier notifier;
    });
}

TEST(UTCFileEventNotifier, WatchDepthAndQueue)
{
    CFileEventNotifier notifier;
    notifier.setWatchDepth(2);
    EXPECT_TRUE(notifier.stillWatching());

    notifier.clearEventQueue();
    notifier.stopEventGeneration();

    IApprise::Event evt;
    notifier.getNextEvent(evt);
    EXPECT_EQ(evt.id, IApprise::Event_none);
    EXPECT_TRUE(evt.message.empty());
}

TEST(UTCFileEventNotifier, AddAndRemoveWatch)
{
    std::string testDir = "/tmp/notifier_watch_dir";
    if (!CFile::exists(testDir))
    {
        CFile::createDirectory(testDir);
    }

    CFileEventNotifier notifier;
    EXPECT_NO_THROW(notifier.addWatch(testDir));
    EXPECT_NO_THROW(notifier.removeWatch(testDir));

    CFile::remove(testDir);
}
