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

// Wii U backend for wtf/StackBounds.cpp. Upstream StackBounds.cpp has no branch
// for this platform (it ends in "#error Need a way to get the stack bounds on
// this platform"), and the OS(UNIX) path relies on pthread_getattr_np /
// pthread_attr_getstack which devkitPPC does not provide. Instead we query the
// stack extent through the WKC peer, which the app implements on top of the
// same handle model our Thread class uses (a pthread_t reinterpreted to void*,
// exactly as ThreadingWKC.cpp passes to wkcThread*Peer).

#include "config.h"
#include <wtf/StackBounds.h>

#include <pthread.h>
#include <wkc/wkcpeer.h>

namespace WTF {

// The peer reports the stack's base (its lowest address) and its size in bytes.
// The stack grows downward on PowerPC, so the high end (origin) is base + size
// and the low end (bound) is base -- matching the origin/bound contract the rest
// of WTF/JSC expects (see the POSIX implementation).
StackBounds StackBounds::newThreadStackBounds(PlatformThreadHandle thread)
{
    void* handle = reinterpret_cast<void*>(thread);
    void* bound = wkcThreadGetStackBasePeer(handle);
    size_t size = wkcThreadGetStackSizePeer(handle);
    void* origin = static_cast<char*>(bound) + size;
    return StackBounds { origin, bound };
}

StackBounds StackBounds::currentThreadStackBoundsInternal()
{
    return newThreadStackBounds(pthread_self());
}

} // namespace WTF
