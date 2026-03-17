#include "config.h"
#include <wtf/RunLoop.h>

namespace WTF {

RunLoop::RunLoop()
{
}

RunLoop::~RunLoop()
{
    m_currentIteration.clear();
}

void RunLoop::run()
{
    // WKC is driven externally by the embedder. RunLoop::run() is not used
    // for the main thread — it exists only to satisfy the interface.
}

void RunLoop::stop()
{
    m_stopped = true;
}

void RunLoop::wakeUp()
{
    // No cross-thread wakeup needed on WKC bare metal.
    // Dispatch will be handled within the embedder's tick.
    dispatch([] { });
}

RunLoop::CycleResult RunLoop::cycle(RunLoopMode)
{
    RunLoop::current().performWork();
    return CycleResult::Continue;
}

} // namespace WTF
