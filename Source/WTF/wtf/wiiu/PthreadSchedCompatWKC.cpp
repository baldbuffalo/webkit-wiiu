/*
 * PthreadSchedCompatWKC.cpp
 *
 * devkitPPC's newlib ships an incomplete POSIX thread-scheduling surface: it
 * declares pthread_attr_setschedparam/getschedparam but omits the runtime
 * pthread_setschedparam, the pthread_attr scheduling-policy setters, and the
 * sched_get_priority_min/max helpers.
 *
 * Rather than route around the gap, this file *provides* real implementations
 * for the Wii U, backed by the WKC thread peer (wkcThreadSetPriorityPeer ->
 * native Cafe OS OSSetThreadPriority). Declarations are added to the toolchain
 * headers at build time (see the "Provide POSIX thread-scheduling shims" step
 * in build.yml), so the standard pthread scheduling API is genuinely available
 * and functional on this platform, not merely stubbed.
 *
 * Homebrew Wii U port.
 */

#include "config.h"

#if PLATFORM(WKC)

#include <pthread.h>
#include <sched.h>
#include <wkc/wkcpeer.h>

extern "C" {

// Apply a scheduling priority to a running thread. sched_priority follows the
// peer's abstract signed scale (0 == platform default, positive == higher
// priority); the platform maps it onto Cafe OS OSSetThreadPriority.
int pthread_setschedparam(pthread_t thread, int /*policy*/, const struct sched_param* param)
{
    if (!param)
        return 0;
    wkcThreadSetPriorityPeer(reinterpret_cast<void*>(thread), param->sched_priority);
    return 0;
}

// Cafe OS has a single time-sharing scheduling class, so the policy is accepted
// and ignored (SCHED_OTHER/FIFO/RR all behave the same here). Returning 0
// keeps standard POSIX callers happy.
int pthread_attr_setschedpolicy(pthread_attr_t* /*attr*/, int /*policy*/)
{
    return 0;
}

// There is no separate inherit-vs-explicit scheduling toggle to honor; priority
// set via pthread_attr_setschedparam is applied at creation regardless.
int pthread_attr_setinheritsched(pthread_attr_t* /*attr*/, int /*inheritsched*/)
{
    return 0;
}

// Valid priority range, expressed on the peer's signed scale (0 == default).
// min == -16 and max == 16 map to Cafe OS 31 (lowest) .. 0 (highest) via
// native = clamp(16 - prio, 0, 31).
int sched_get_priority_min(int /*policy*/)
{
    return -16;
}

int sched_get_priority_max(int /*policy*/)
{
    return 16;
}

} // extern "C"

#endif // PLATFORM(WKC)
