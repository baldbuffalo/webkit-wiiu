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
}

void RunLoop::wakeUp()
{
}

RunLoop::CycleResult RunLoop::cycle(RunLoopMode)
{
    return CycleResult::Continue;
}

} // namespace WTF
