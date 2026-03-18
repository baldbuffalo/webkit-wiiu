/*
 * Copyright (c) 2010-2011 ACCESS CO., LTD. All rights reserved.
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

#pragma once

#include <wtf/text/CString.h>

namespace WKC {

class WKCFontInfo {
public:
    ~WKCFontInfo();

    static WKCFontInfo* create(float size, int weight, bool italic, bool horizontal, bool verticalright, int family, const char* familyName);
    static WKCFontInfo* copy(const WKCFontInfo* other);

    bool isEqual(const WKCFontInfo* other);
    const WTF::CString& familyName() const { return m_familyName; }

    void* font() const { return m_font; }
    float scale() const { return m_scale; }
    float iscale() const { return m_iscale; }
    float requestSize() const { return m_requestSize; }
    float createdSize() const { return m_createdSize; }
    int weight() const { return m_weight; }
    bool isItalic() const { return m_isItalic; }
    bool canScale() const { return m_canScale; }
    float ascent() const { return m_ascent; }
    float descent() const { return m_descent; }
    float lineSpacing() const { return m_lineSpacing; }

    void setSpecificUnicodeChar(int ch) { m_unicodeChar = ch; }
    int specificUnicodeChar() const { return m_unicodeChar; }

    bool horizontal() const { return m_horizontal; }

private:
    WKCFontInfo(const char* familyName);
    bool construct(float size, int weight, bool italic, bool horizontal, bool verticalright, int family);

    void* m_font { nullptr };
    float m_scale { 1.f };
    float m_iscale { 1.f };
    float m_requestSize { 0.f };
    float m_createdSize { 0.f };
    int m_weight { 0 };
    bool m_isItalic { false };
    bool m_canScale { false };
    float m_ascent { 0.f };
    float m_descent { 0.f };
    float m_lineSpacing { 0.f };
    int m_unicodeChar { 0 };
    bool m_horizontal { true };

    WTF::CString m_familyName;
};

} // namespace WKC
