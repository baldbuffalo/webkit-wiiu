/*
 * Copyright (c) 2011, 2014 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "FontCustomPlatformData.h"
#include "FontPlatformDataWKC.h"

#include "FontDescription.h"
#include "FontPlatformData.h"
#include "SharedBuffer.h"

#include <wtf/text/CString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

FontCustomPlatformData::FontCustomPlatformData(SharedBuffer& buffer)
    : m_buffer(buffer)
    , m_registeredId(-1)
{
    ::memset(m_familyName, 0, sizeof(m_familyName));
}

FontCustomPlatformData::~FontCustomPlatformData()
{
    if (m_registeredId >= 0)
        wkcFontEngineUnregisterFontPeer(m_registeredId);
}

std::unique_ptr<FontCustomPlatformData> FontCustomPlatformData::create(SharedBuffer& buffer, String&)
{
    auto self = std::unique_ptr<FontCustomPlatformData>(new FontCustomPlatformData(buffer));
    if (!self->construct())
        return nullptr;
    return self;
}

bool FontCustomPlatformData::construct()
{
    auto contiguous = m_buffer->makeContiguous();
    const auto* data = contiguous->data();
    size_t size = contiguous->size();

    m_registeredId = wkcFontEngineRegisterFontPeer(
        WKC_FONT_ENGINE_REGISTER_TYPE_MEMORY,
        reinterpret_cast<const unsigned char*>(data),
        size);
    if (m_registeredId < 0)
        return false;

    return wkcFontEngineGetFamilyNamePeer(m_registeredId, m_familyName, sizeof(m_familyName));
}

FontPlatformData FontCustomPlatformData::fontPlatformData(const FontDescription& desc, bool bold, bool italic, float size)
{
    FontPlatformData fd(desc, AtomString::fromUTF8(m_familyName));
    if (fd.font() && fd.font()->font())
        return fd;
    return FontPlatformData(desc, "systemfont"_s);
}

bool FontCustomPlatformData::supportsFormat(const String& format)
{
    return wkcFontEngineCanSupportByFormatNamePeer(
        reinterpret_cast<const unsigned char*>(format.utf8().data()),
        format.utf8().length());
}

} // namespace WebCore
