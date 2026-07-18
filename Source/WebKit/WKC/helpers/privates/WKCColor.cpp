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
#include "Color.h"
#include "helpers/WKCColor.h"

namespace WKC {

// WKC::Color is a thin handle owning a heap-allocated WebCore::Color. ColorPrivate
// is only an opaque tag in the public header; internally the handle is exactly a
// WebCore::Color*. These typed helpers replace the old C-cast PARENT() macro so
// the reinterpretation happens in exactly one place and reads clearly.
static inline WebCore::Color* toColor(ColorPrivate* p) { return reinterpret_cast<WebCore::Color*>(p); }
static inline const WebCore::Color* toColor(const ColorPrivate* p) { return reinterpret_cast<const WebCore::Color*>(p); }
static inline ColorPrivate* toPrivate(WebCore::Color* c) { return reinterpret_cast<ColorPrivate*>(c); }

Color::Color()
    : m_parent(toPrivate(new WebCore::Color()))
{
}

Color::Color(ColorPrivate* parent)
    : m_parent(toPrivate(new WebCore::Color(*toColor(parent))))
{
}

Color::Color(const Color& other)
    : m_parent(toPrivate(new WebCore::Color(*toColor(other.parent()))))
{
}

Color::~Color()
{
    delete toColor(m_parent);
}

Color&
Color::operator=(const Color& other)
{
    if (this != &other)
        *toColor(m_parent) = *toColor(other.parent());
    return *this;
}

bool
Color::isValid() const
{
    return toColor(m_parent)->isValid();
}

RGBA32
Color::rgb() const
{
    // Modern WebCore::Color is no longer a packed integer (it can hold wide-gamut
    // or out-of-line color data). Flatten it losslessly-enough to 8-bit sRGB and
    // repack as legacy ARGB (0xAARRGGBB) -- the historical meaning of RGBA32.
    return WebCore::PackedColor::ARGB { toColor(m_parent)->toColorTypeLossy<WebCore::SRGBA<uint8_t>>() }.value;
}

} // namespace WKC
