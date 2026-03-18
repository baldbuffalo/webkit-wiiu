/*
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
#include "FontCache.h"

#include "Font.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "FontPlatformDataWKC.h"
#include <wtf/text/AtomString.h>

namespace WebCore {

void FontCache::platformInit()
{
}

Vector<String> FontCache::systemFontFamilies()
{
    return { };
}

bool FontCache::isSystemFontForbiddenForEditing(const String&)
{
    return false;
}

std::optional<ASCIILiteral> FontCache::platformAlternateFamilyName(const String&)
{
    return std::nullopt;
}

FontPlatformData* FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomString& family, float size, const FontCreationContext&)
{
    auto* ret = new FontPlatformData(fontDescription, family);
    if (!ret || !ret->font() || (!ret->font()->font() && family != "nullfont"_s)) {
        delete ret;
        return nullptr;
    }
    return ret;
}

RefPtr<Font> FontCache::lastResortFallbackFont(const FontDescription& fontDescription)
{
    auto data = fontForFamily(fontDescription, "systemfont"_s);
    if (!data)
        data = fontForFamily(fontDescription, "nullfont"_s);
    return data;
}

} // namespace WebCore
