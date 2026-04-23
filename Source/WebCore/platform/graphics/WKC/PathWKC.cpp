/*
 *  Copyright (C) 2007-2009 Torch Mobile, Inc.
 *  Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
 *
 *  [license omitted]
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

// NOTE: The following Path methods are inline in Path.h and delegate directly
// to m_data (PlatformPathWKC value member) — do NOT redefine them here:
//   moveTo, addLineTo, addQuadCurveTo, addBezierCurveTo, addArcTo,
//   closeSubpath, addRect, isEmpty, currentPoint
//
// Path() = default and Path(const Path&) are also handled by the compiler.

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

void Path::addArc(const FloatPoint& p, float radius, float startAngle, float endAngle, RotationDirection direction)
{
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return;
    path->addEllipse(p, radius, radius, startAngle, endAngle,
        direction == RotationDirection::Counterclockwise);
}

void Path::addEllipse(const FloatPoint& p, float radiusX, float radiusY, float /*rotation*/,
    float startAngle, float endAngle, RotationDirection direction)
{
    // rotation is not supported in WKC; approximate with axis-aligned ellipse
    auto* path = static_cast<PlatformPathWKC*>(platformPath());
    if (!path) return;
    path->addEllipse(p, radiusX, radiusY, startAngle, endAngle,
        direction == RotationDirection::Counterclockwise);
}

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
