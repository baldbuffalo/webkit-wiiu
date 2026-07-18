/*
 * Copyright (c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include <wtf/URL.h>
// 2026: LinkHash.h / WebCore::visitedLinkHash() were replaced by
// SharedStringHash.h / WebCore::computeSharedStringHash().
#include "SharedStringHash.h"
#include "helpers/WKCLinkHash.h"

namespace WKC {

LinkHash
visitedLinkHash(const unsigned short* url, unsigned length)
{
    return WebCore::computeSharedStringHash(std::span<const char16_t> { reinterpret_cast<const char16_t*>(url), length });
}

LinkHash
visitedLinkHash(const char* url)
{
    WTF::URL kurl(WTF::URL(), WTF::String::fromUTF8(url));
    return WebCore::computeSharedStringHash(kurl.string());
}

} // namespace
