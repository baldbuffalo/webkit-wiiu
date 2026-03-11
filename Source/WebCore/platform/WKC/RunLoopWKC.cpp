/*
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include <wtf/RunLoop.h>

namespace WTF {

RunLoop::RunLoop()
{
}

RunLoop::~RunLoop()
{
    m_currentIteration = nullptr;
}

void RunLoop::run()
{
    // WKC is a single-threaded embedder. The main loop is driven externally
    // by the embedder calling wkc_tick() each frame. RunLoop::run() is not
    // used for the main thread; it exists only to satisfy the interface.
}

void RunLoop::stop()
{
    if (m_currentIteration)
        m_currentIteration->stop();
}

void RunLoop::wakeUp()
{
    // No cross-thread wakeup needed on WKC — dispatch tasks are handled
    // synchronously within the embedder's tick.
    scheduleDispatch();
}

RunLoop::CycleResult RunLoop::cycle(RunLoopMode)
{
    performWork();
    return CycleResult::Continue;
}

} // namespace WTF
