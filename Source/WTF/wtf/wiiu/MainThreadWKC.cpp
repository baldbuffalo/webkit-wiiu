/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
 * Modernized for current WebKit — webkit-wiiu.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MainThread.h"

// Modern WebKit dispatches main-thread functions through RunLoop, not through
// the old scheduleDispatchFunctionsOnMainThread()/dispatchFunctionsFromMainThread()
// hooks (which were removed). The only platform hook left is
// initializeMainThreadPlatform(); main-thread scheduling lives in the RunLoop
// implementation.

namespace WTF {

void initializeMainThreadPlatform()
{
}

} // namespace WTF
