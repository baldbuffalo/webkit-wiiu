#pragma once
// Force-complete ScheduledTask before RunLoop.h uses CheckedPtr on it
#if defined(__cplusplus)
namespace WTF {
class RunLoop;
class RunLoopTimerBase {
public:
    class ScheduledTask {
    public:
        void incrementCheckedPtrCount() const { }
        void decrementCheckedPtrCount() const { }
    };
};
} // namespace WTF
#endif
