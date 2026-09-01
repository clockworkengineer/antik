#ifndef IFILEEVENTNOTIFIER_HPP
#define IFILEEVENTNOTIFIER_HPP
//
// C++ STL
//
#include <string>
#include <stdexcept>
//
// Antik classes
//
#include "IApprise.hpp"
// =========
// NAMESPACE
// =========
namespace Antik::File
{
    // ================
    // CLASS DEFINITION
    // ================
    class IFileEventNotifier
    {
    public:
        virtual ~IFileEventNotifier() = default;

        //
        // Event queue
        //
        virtual void generateEvents(void) = 0;                   // Watch folder(s) for file events
        virtual void stopEventGeneration(void) = 0;              // Stop watch loop/thread
        virtual void getNextEvent(IApprise::Event &message) = 0; // Get next queued event
        virtual bool stillWatching() const = 0;                  // Events still being generated
        virtual void clearEventQueue() = 0;                      // Clear event queue
        //
        // Watch processing
        //
        virtual void addWatch(const std::string &filePath) = 0;    // Add path to be watched
        virtual void removeWatch(const std::string &filePath) = 0; // Remove path being watched
        virtual void setWatchDepth(int watchDepth)                 // Set maximum watch depth (LSP default)
        {
            (void)watchDepth;
        }
        //
        // Get any thrown exceptions
        //
        virtual std::exception_ptr getThrownException() const = 0;
    };

    //
    // Extended Watchable Event Notifier Interface (LSP compliance)
    //
    class IWatchableEventNotifier : public IFileEventNotifier
    {
    public:
        virtual ~IWatchableEventNotifier() = default;
    };
} // namespace Antik::File
#endif /* IFILEEVENTNOTIFIER_HPP */
