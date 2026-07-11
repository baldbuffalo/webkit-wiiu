/*
 *  WKCLocalStorageUtil.cpp
 *
 *  Copyright (c) 2014 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"

#include "WKCLocalStorageUtil.h"

// NOTE: WebCore::StorageTracker (the per-origin localStorage usage/quota tracker)
// was removed from modern WebKit — origin accounting moved out of the engine core
// into the storage/network process, i.e. the embedder layer. localStorage itself
// still works through the modern storage layer; only this administrative tracking
// API lost its in-engine backend.
//
// These public WKCWebKit* entry points are kept as safe no-ops so wave-browser
// continues to link against them. If per-origin storage management is needed
// later, it is implemented at the app layer where WebKit now tracks it.

namespace WKC {

WKC_DEFINE_GLOBAL_PTR(const LocalStorageTrackerCallbacks*, gStorageTrackerCallbacks, 0);

void
WKCWebKitInitializeLocalStorageTracker(const char* path, const LocalStorageTrackerCallbacks* callbacks)
{
    // Retain the callbacks for API compatibility; there is no engine-side tracker
    // to register them with anymore.
    (void)path;
    gStorageTrackerCallbacks = callbacks;
}

unsigned int
WKCWebKitGetLocalStorageOriginsNum()
{
    return 0;
}

int
WKCWebKitGetLocalStorageOrigin(int, char* buf, int length)
{
    if (buf && length > 0)
        buf[0] = '\0';
    return 0;
}

long long
WKCWebKitGetLocalStorageDiskUsageForOrigin(const char*)
{
    return 0;
}

void
WKCWebKitDeleteLocalStorageOrigin(const char*)
{
}

void
WKCWebKitDeleteAllLocalStorageOrigins()
{
}

} // namespace
