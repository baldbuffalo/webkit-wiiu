/*
 *  Copyright (C) 2007-2009 Torch Mobile, Inc.
 *  Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"

#if !USE(WKC_CAIRO)

#include "Path.h"
#include "AffineTransform.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "PlatformPathWKC.h"
#include <wtf/MathExtras.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

// NOTE: The following Path methods are inline in Path.h and must NOT be
// redefined here. They delegate to m_data (PlatformPathWKC value member):
//   Path() = default, Path(const Path&), ~Path(), operator=
//   moveTo, addLineTo, addQuadCurveTo, addBezierCurveTo, addArcTo,
//   closeSubpath, addRect, addArc, addEllipse, isEmpty, currentPoint

bool Path::contains(const FloatPoint& point, WindRule rule) const
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return false;
    return path->contains(point, rule);
}

void Path::translate(const FloatSize& size)
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return;
    path->translate(size);
}

FloatRect Path::boundingRect() const
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return FloatRect();
    return path->boundingRect();
}

void Path::clear()
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return;
    path->clear();
}

void Path::transform(const AffineTransform& t)
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return;
    path->transform(t);
}

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
