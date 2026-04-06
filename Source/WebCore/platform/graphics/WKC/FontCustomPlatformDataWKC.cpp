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
#include <WebCore/FontCustomPlatformData.h>
#include "FontCreationContext.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "SharedBuffer.h"

#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/CString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

struct WKCFontRegistration {
    int registeredId { -1 };
    String familyName;
    RefPtr<SharedBuffer> buffer;
    String itemInCollection;
};

static HashMap<uint64_t, WKCFontRegistration>& fontRegistrations()
{
    static NeverDestroyed<HashMap<uint64_t, WKCFontRegistration>> map;
    return map;
}

RefPtr<FontCustomPlatformData> FontCustomPlatformData::create(SharedBuffer& buffer, const String& itemInCollection)
{
    auto contiguous = buffer.makeContiguous();
    auto span = contiguous->span();
    const auto* data = reinterpret_cast<const unsigned char*>(span.data());
    size_t size = span.size();

    if (!data || !size)
        return nullptr;

    if (!wkcFontEngineCanSupportPeer(data, size))
        return nullptr;

    int registeredId = wkcFontEngineRegisterFontPeer(
        WKC_FONT_ENGINE_REGISTER_TYPE_MEMORY, data, size);
    if (registeredId < 0)
        return nullptr;

    char familyNameBuf[128] = { };
    if (!wkcFontEngineGetFamilyNamePeer(registeredId, familyNameBuf, sizeof(familyNameBuf))) {
        wkcFontEngineUnregisterFontPeer(registeredId);
        return nullptr;
    }

    WKCFontRegistration reg;
    reg.registeredId = registeredId;
    reg.familyName = String::fromUTF8(familyNameBuf);
    reg.buffer = &buffer;
    reg.itemInCollection = itemInCollection;

    FontPlatformData::CreationData creationData { buffer.makeContiguous(), itemInCollection };
    auto self = adoptRef(*new FontCustomPlatformData{ WTFMove(creationData), RenderingResourceIdentifier::generate() });
    uint64_t key = self->m_renderingResourceIdentifier.toUInt64();
    fontRegistrations().set(key, WTFMove(reg));

    return self;
}

RefPtr<FontCustomPlatformData> FontCustomPlatformData::createMemorySafe(SharedBuffer& buffer, const String& itemInCollection)
{
    return create(buffer, itemInCollection);
}

FontCustomPlatformData::~FontCustomPlatformData()
{
    uint64_t key = m_renderingResourceIdentifier.toUInt64();
    auto it = fontRegistrations().find(key);
    if (it != fontRegistrations().end()) {
        if (it->value.registeredId >= 0)
            wkcFontEngineUnregisterFontPeer(it->value.registeredId);
        fontRegistrations().remove(it);
    }
}

FontPlatformData FontCustomPlatformData::fontPlatformData(const FontDescription& desc, bool, bool, const FontCreationContext&)
{
    return FontPlatformData(
        desc.computedSize(),
        false,
        false,
        desc.orientation(),
        desc.widthVariant(),
        desc.textRenderingMode(),
        this
    );
}

bool FontCustomPlatformData::supportsFormat(const String& format)
{
    return wkcFontEngineCanSupportByFormatNamePeer(
        reinterpret_cast<const unsigned char*>(format.utf8().data()),
        format.utf8().length());
}

bool FontCustomPlatformData::supportsTechnology(const FontTechnology&)
{
    return false;
}

FontCustomPlatformSerializedData FontCustomPlatformData::serializedData() const
{
    uint64_t key = m_renderingResourceIdentifier.toUInt64();
    auto it = fontRegistrations().find(key);
    if (it != fontRegistrations().end() && it->value.buffer)
        return { Ref { *it->value.buffer }, it->value.itemInCollection, m_renderingResourceIdentifier };
    return { SharedBuffer::create(), String(), m_renderingResourceIdentifier };
}

std::optional<Ref<FontCustomPlatformData>> FontCustomPlatformData::tryMakeFromSerializationData(FontCustomPlatformSerializedData&& data, bool)
{
    auto result = create(data.fontFaceData, data.itemInCollection);
    if (!result)
        return std::nullopt;
    return Ref { *result };
}

} // namespace WebCore
