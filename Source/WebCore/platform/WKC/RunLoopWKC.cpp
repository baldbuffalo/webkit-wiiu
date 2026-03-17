#include "config.h"
#include <wtf/RunLoop.h>

namespace WTF {

RunLoop::RunLoop()
{
}

RunLoop::~RunLoop()
{
}

void RunLoop::run()
{
}

void RunLoop::stop()
{
    // Locker locker { m_loopLock };
    // m_shutdown = true;
    // wakeUpWithLock();
}

void RunLoop::wakeUp()
{
    // Bare metal — no cross-thread wakeup needed.
    // Locker locker { m_loopLock };
    // wakeUpWithLock();
}

RunLoop::CycleResult RunLoop::cycle(RunLoopMode)
{
    return CycleResult::Continue;
}

} // namespace WTF
