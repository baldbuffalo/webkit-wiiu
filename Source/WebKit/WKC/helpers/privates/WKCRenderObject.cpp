/*
 * Copyright (c) 2011-2015 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCRenderObject.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"
#include "helpers/privates/WKCRenderStylePrivate.h"

#include "RenderObject.h"
#include "RenderObjectInlines.h"
#include "RenderObjectStyle.h"
#include "StyleComputedStyle.h"
#include "StyleComputedStyle+GettersInlines.h"

#include "FloatQuad.h"
#include "LayoutRect.h"

namespace WKC {
RenderObjectPrivate::RenderObjectPrivate(WebCore::RenderObject* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

RenderObjectPrivate::~RenderObjectPrivate() = default;

bool
RenderObjectPrivate::isTextControl() const
{
    return m_webcore->isRenderTextControl();
}

bool
RenderObjectPrivate::isTextArea() const
{
    // Textareas render as multi-line text controls (RenderTextControlMultiLine).
    return m_webcore->isRenderTextControlMultiLine();
}

WKCRect
RenderObjectPrivate::absoluteBoundingBoxRect(bool usetransform)
{
    return m_webcore->absoluteBoundingBoxRect(usetransform);
}

void
RenderObjectPrivate::focusRingRects(WTF::Vector<WKCRect>& rects)
{
    // focusRingRects(Vector<IntRect>&) was replaced by the quad-based
    // absoluteFocusRingQuads(Vector<FloatQuad>&); flatten each quad to its
    // enclosing bounding box to preserve the historical rect list.
    WTF::Vector<WebCore::FloatQuad> quads;
    m_webcore->absoluteFocusRingQuads(quads);
    rects.grow(quads.size());
    for (size_t i = 0; i < quads.size(); ++i)
        rects[i] = quads[i].enclosingBoundingBox();
}

WKCRect
RenderObjectPrivate::absoluteClippedOverflowRect()
{
    return WebCore::enclosingIntRect(m_webcore->absoluteClippedOverflowRectForRepaint());
}

bool
RenderObjectPrivate::hasOutline() const
{
    return m_webcore->style().hasOutline();
}

RenderStyle*
RenderObjectPrivate::style()
{
    const WebCore::Style::ComputedStyle& renderStyle = m_webcore->style();
    if (!m_renderStyle || m_renderStyle->webcore() != &renderStyle)
        m_renderStyle = std::make_unique<RenderStylePrivate>(&renderStyle);
    return &m_renderStyle->wkc();
}

RenderObject::RenderObject(RenderObjectPrivate& parent)
    : m_private(parent)
{
}

RenderObject::~RenderObject()
{
}


bool
RenderObject::isTextControl() const
{
    return m_private.isTextControl();
}

bool
RenderObject::isTextArea() const
{
    return m_private.isTextArea();
}

WKCRect
RenderObject::absoluteBoundingBoxRect(bool usetransform) const
{
    return m_private.absoluteBoundingBoxRect(usetransform);
}

WKCRingRects *
RenderObject::focusRingRects() const
{
    WTF::Vector<WKCRect> core_rects;
    m_private.focusRingRects(core_rects);
    WKCRingRectsPrivate *p = new WKCRingRectsPrivate(core_rects);
    return new WKCRingRects(p);
}

WKCRect
RenderObject::absoluteClippedOverflowRect()
{
    return m_private.absoluteClippedOverflowRect();
}

bool
RenderObject::hasOutline() const
{
    return m_private.hasOutline();
}

RenderStyle*
RenderObject::style() const
{
    return m_private.style();
}


WKCRingRectsPrivate::WKCRingRectsPrivate(WTF::Vector<WKCRect>& rects)
    : m_rects(rects)
{
}
WKCRingRectsPrivate::~WKCRingRectsPrivate() = default;
int
WKCRingRectsPrivate::length() const
{
    return m_rects.size();
}
WKCRect
WKCRingRectsPrivate::getAt(int i) const
{
    return m_rects.at(i);
}

void
WKCRingRects::destroy(WKCRingRects *self)
{
    delete self;
}

WKCRingRects::WKCRingRects(WKCRingRectsPrivate* p)
    : m_private(p)
{
}
WKCRingRects::~WKCRingRects()
{
    delete m_private;
}
int
WKCRingRects::length() const
{
    return m_private->length();
}
WKCRect
WKCRingRects::getAt(int i) const
{
    return m_private->getAt(i);
}

} // namespace
