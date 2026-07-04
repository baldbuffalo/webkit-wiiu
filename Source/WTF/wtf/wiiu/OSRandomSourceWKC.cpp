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

// Wii U backend for the OS randomness source. Upstream reaches its randomness
// through RandomDevice, whose header ends in
// "#error This configuration doesn't have a strong source of randomness." for
// platforms that are neither Darwin/Fuchsia/Windows nor OS(UNIX) with
// /dev/urandom. The Wii U has neither /dev/urandom nor those OS APIs, so we
// bypass RandomDevice entirely and fill the buffer straight from the WKC
// randomness peer (which the app is required to back with a cryptographically
// strong source). Both wtf/RandomDevice.cpp and wtf/OSRandomSource.cpp are
// excluded from the build in favour of this file.

#include "config.h"
#include <wtf/OSRandomSource.h>

#include <wkc/wkcpeer.h>

namespace WTF {

void cryptographicallyRandomValuesFromOS(std::span<uint8_t> buffer)
{
    if (buffer.empty())
        return;
    wkcRandomNumbersPeer(reinterpret_cast<unsigned char*>(buffer.data()), static_cast<int>(buffer.size()));
}

} // namespace WTF
