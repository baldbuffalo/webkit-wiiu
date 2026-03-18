/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FontPlatformData.h"
#include "FontPlatformDataWKC.h"

#include "FontDescription.h"
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

static bool gEnableScalingMonosizeFont = false;

namespace WebCore {

void FontPlatformData_enableScalingMonosizeFont(bool flag)
{
    gEnableScalingMonosizeFont = flag;
}

static WKC::WKCFontInfo* _createWKCFont(const FontDescription& desc, const char* familyName)
{
    float size = floorf(desc.computedSize() + 0.5f);
    bool italic = desc.isItalic();
    int family = WKC_FONT_FAMILY_NONE;
    int weight = WKC_FONT_WEIGHT_NORMAL;

    switch (desc.genericFamily()) {
    case FontDescription::NoFamily:        family = WKC_FONT_FAMILY_NONE;       break;
    case FontDescription::StandardFamily:  family = WKC_FONT_FAMILY_STANDARD;   break;
    case FontDescription::SerifFamily:     family = WKC_FONT_FAMILY_SERIF;      break;
    case FontDescription::SansSerifFamily: family = WKC_FONT_FAMILY_SANSSERIF;  break;
    case FontDescription::MonospaceFamily: family = WKC_FONT_FAMILY_MONOSPACE;  break;
    case FontDescription::CursiveFamily:   family = WKC_FONT_FAMILY_CURSIVE;    break;
    case FontDescription::FantasyFamily:   family = WKC_FONT_FAMILY_FANTASY;    break;
    case FontDescription::PictographFamily:family = WKC_FONT_FAMILY_PICTOGRAPH; break;
    }

    int w = (int)desc.weight();
    if      (w < 150) weight = WKC_FONT_WEIGHT_100;
    else if (w < 250) weight = WKC_FONT_WEIGHT_200;
    else if (w < 350) weight = WKC_FONT_WEIGHT_300;
    else if (w < 450) weight = WKC_FONT_WEIGHT_400;
    else if (w < 550) weight = WKC_FONT_WEIGHT_500;
    else if (w < 650) weight = WKC_FONT_WEIGHT_600;
    else if (w < 750) weight = WKC_FONT_WEIGHT_700;
    else if (w < 850) weight = WKC_FONT_WEIGHT_800;
    else              weight = WKC_FONT_WEIGHT_900;

    bool horizontal = desc.orientation() == FontOrientation::Horizontal;
    bool verticalright = desc.textOrientation() == TextOrientation::Mixed;

    return WKC::WKCFontInfo::create(size, weight, italic, horizontal, verticalright, family, familyName);
}

FontPlatformData::FontPlatformData(const FontDescription& desc, const AtomString& family)
    : FontPlatformData(desc.computedSize(), desc.syntheticBold(), desc.syntheticOblique(),
                       desc.orientation(), desc.widthVariant(), desc.textRenderingMode())
{
    if (m_creationData)
        m_creationData->font = _createWKCFont(desc, family.string().utf8().data());
    else {
        m_creationData = CustomFontCreationData { _createWKCFont(desc, family.string().utf8().data()) };
    }
}

FontPlatformData::FontPlatformData(const FontDescription& desc, const char* familyName)
    : FontPlatformData(desc.computedSize(), desc.syntheticBold(), desc.syntheticOblique(),
                       desc.orientation(), desc.widthVariant(), desc.textRenderingMode())
{
    m_creationData = CustomFontCreationData { _createWKCFont(desc, familyName) };
}

WKC::WKCFontInfo* FontPlatformData::font() const
{
    return m_creationData ? m_creationData->font : nullptr;
}

} // namespace WebCore
