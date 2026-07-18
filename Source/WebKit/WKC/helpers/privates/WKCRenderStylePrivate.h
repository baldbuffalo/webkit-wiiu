/*
 * Copyright (c) 2015 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_PRIVATE_RENDERSTYLE_H_
#define _WKC_HELPERS_PRIVATE_RENDERSTYLE_H_

#include "helpers/WKCRenderStyle.h"

namespace WebCore {
// Modern WebCore renamed RenderStyle to Style::ComputedStyle; the WKC public
// wrapper keeps the historical RenderStyle facade name.
namespace Style {
class ComputedStyle;
} // namespace Style
} // namespace WebCore

namespace WKC {

class RenderStyleWrap : public RenderStyle {
public:
    RenderStyleWrap(RenderStylePrivate& parent) : RenderStyle(parent) {}
    ~RenderStyleWrap() = default;
};

class RenderStylePrivate {
public:
    explicit RenderStylePrivate(const WebCore::Style::ComputedStyle*);
    ~RenderStylePrivate() = default;

    const WebCore::Style::ComputedStyle* webcore() const { return m_webcore; }
    RenderStyle& wkc() { return m_wkc; }

#if ENABLE(TOUCH_EVENTS)
    Color tapHighlightColor() const;
#endif

private:
    const WebCore::Style::ComputedStyle* m_webcore;
    RenderStyleWrap m_wkc;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_RENDERSTYLE_H_

