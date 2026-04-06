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
#include "WebCore/FontPlatformSerializedData.h"
#include "FontPlatformData.h"
#include "FontPlatformDataWKC.h"

#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

static bool gEnableScalingMonosizeFont = false;

namespace WebCore {

void FontPlatformData_enableScalingMonosizeFont(bool flag)
{
    gEnableScalingMonosizeFont = flag;
}

// ─── WKC font registry keyed by FontPlatformData hash ───────────────────────

static HashMap<unsigned, WKC::WKCFontInfo*>& wkcFontMap()
{
    static NeverDestroyed<HashMap<unsigned, WKC::WKCFontInfo*>> map;
    return map;
}

WKC::WKCFontInfo* wkcGetFont(const FontPlatformData& d)
{
    auto it = wkcFontMap().find(d.hash());
    if (it != wkcFontMap().end())
        return it->value;
    return nullptr;
}

void wkcRegisterFont(unsigned hash, WKC::WKCFontInfo* info)
{
    wkcFontMap().set(hash, info);
}

void wkcUnregisterFont(unsigned hash)
{
    auto it = wkcFontMap().find(hash);
    if (it != wkcFontMap().end()) {
        delete it->value;
        wkcFontMap().remove(it);
    }
}

} // namespace WebCore

// ─── WKCFontInfo implementation ─────────────────────────────────────────────

namespace WKC {

WKCFontInfo::WKCFontInfo(const char* familyName)
    : m_familyName(familyName)
{
}

WKCFontInfo::~WKCFontInfo()
{
    if (m_font)
        wkcFontDeletePeer(m_font);
}

WKCFontInfo* WKCFontInfo::create(float size, int weight, bool italic, bool horizontal, bool verticalright, int family, const char* familyName)
{
    WKCFontInfo* self = new WKCFontInfo(familyName);
    if (!self->construct(size, weight, italic, horizontal, verticalright, family)) {
        delete self;
        return nullptr;
    }
    return self;
}

bool WKCFontInfo::construct(float size, int weight, bool italic, bool horizontal, bool verticalright, int family)
{
    m_font = wkcFontNewPeer((int)size, weight, italic, horizontal, verticalright, family, m_familyName.data());
    m_scale = 1.f;
    m_requestSize = size;
    m_createdSize = m_font ? wkcFontGetSizePeer(m_font) : m_requestSize;
    if (gEnableScalingMonosizeFont && m_createdSize != m_requestSize)
        m_scale = m_requestSize / m_createdSize;
    m_iscale = 1.f / m_scale;
    m_weight = weight;
    m_isItalic = italic;
    if (m_font) {
        m_canScale = wkcFontCanScalePeer(m_font);
        m_ascent = wkcFontGetAscentPeer(m_font);
        m_descent = wkcFontGetDescentPeer(m_font);
        m_lineSpacing = wkcFontGetLineSpacingPeer(m_font);
    }
    m_unicodeChar = 0;
    m_horizontal = horizontal;
    return true;
}

WKCFontInfo* WKCFontInfo::copy(const WKCFontInfo* other)
{
    WKCFontInfo* self = new WKCFontInfo(other->m_familyName.data());
    self->m_font        = other->m_font ? wkcFontNewCopyPeer(other->m_font) : nullptr;
    self->m_scale       = other->m_scale;
    self->m_iscale      = other->m_iscale;
    self->m_requestSize = other->m_requestSize;
    self->m_createdSize = other->m_createdSize;
    self->m_weight      = other->m_weight;
    self->m_isItalic    = other->m_isItalic;
    self->m_canScale    = other->m_canScale;
    self->m_ascent      = other->m_ascent;
    self->m_descent     = other->m_descent;
    self->m_lineSpacing = other->m_lineSpacing;
    self->m_unicodeChar = other->m_unicodeChar;
    self->m_horizontal  = other->m_horizontal;
    return self;
}

bool WKCFontInfo::isEqual(const WKCFontInfo* other)
{
    if (m_scale != other->m_scale ||
        m_requestSize != other->m_requestSize ||
        m_createdSize != other->m_createdSize ||
        m_weight != other->m_weight ||
        m_isItalic != other->m_isItalic ||
        m_unicodeChar != other->m_unicodeChar)
        return false;
    return wkcFontIsEqualPeer(m_font, other->m_font);
}

} // namespace WKC
