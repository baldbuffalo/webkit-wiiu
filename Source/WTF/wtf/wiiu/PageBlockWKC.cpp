/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

// Wii U backend for wtf/PageBlock.cpp. Upstream defines systemPageSize() only
// for OS(UNIX) (via sysconf) and OS(WINDOWS) (via GetSystemInfo); neither is
// true on devkitPPC, so systemPageSize() is undefined there. We report a fixed
// 4 KiB page granularity: it is a power of two and evenly divides the PowerPC
// CeilingOnPageSize (64 KiB), which pageSize() asserts on.

#include "config.h"
#include <wtf/PageBlock.h>

namespace WTF {

static size_t s_pageSize;

static inline size_t systemPageSize()
{
    return 4096; // 4 KiB
}

size_t pageSize()
{
    if (!s_pageSize) {
        s_pageSize = systemPageSize();
        RELEASE_ASSERT(isPowerOfTwo(s_pageSize));
        RELEASE_ASSERT_WITH_MESSAGE(s_pageSize <= CeilingOnPageSize, "CeilingOnPageSize is too low, raise it in PageBlock.h!");
        RELEASE_ASSERT(roundUpToMultipleOf(s_pageSize, CeilingOnPageSize) == CeilingOnPageSize);
    }
    return s_pageSize;
}

} // namespace WTF
