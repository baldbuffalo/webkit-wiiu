/*
 *  Copyright (C) 2007-2009 Torch Mobile, Inc.
 *  Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  [license omitted]
 */

#include "config.h"

#if !USE(WKC_CAIRO)

#include "FloatRect.h"
#include "GraphicsContext.h"
#include "Path.h"
#include "PlatformPathWKC.h"
#include "TransformationMatrix.h"
#include <wtf/MathExtras.h>
#include <wtf/text/WTFString.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

static inline bool equalAngle(double a, double b)
{
    return fabs(a - b) < 1E-5;
}

static void getEllipsePointByAngle(double angle, double a, double b, float& x, float& y)
{
    while (angle < 0)
        angle += 2 * WTF::piDouble;
    while (angle >= 2 * WTF::piDouble)
        angle -= 2 * WTF::piDouble;

    if (equalAngle(angle, 0) || equalAngle(angle, 2 * WTF::piDouble)) {
        x = a; y = 0;
    } else if (equalAngle(angle, WTF::piDouble)) {
        x = -a; y = 0;
    } else if (equalAngle(angle, .5 * WTF::piDouble)) {
        x = 0; y = b;
    } else if (equalAngle(angle, 1.5 * WTF::piDouble)) {
        x = 0; y = -b;
    } else {
        double k = tan(angle);
        double sqA = a * a;
        double sqB = b * b;
        double tmp = 1. / (1. / sqA + (k * k) / sqB);
        tmp = tmp <= 0 ? 0 : sqrt(tmp);
        if (angle > .5 * WTF::piDouble && angle < 1.5 * WTF::piDouble)
            tmp = -tmp;
        x = tmp;

        k = tan(.5 * WTF::piDouble - angle);
        tmp = 1. / ((k * k) / sqA + 1 / sqB);
        tmp = tmp <= 0 ? 0 : sqrt(tmp);
        if (angle > WTF::piDouble)
            tmp = -tmp;
        y = tmp;
    }
}

static void quadCurve(int segments, Vector<PathPoint>& pts, const PathPoint* control)
{
    const float step = 1.0f / segments;
    float tA = 0.0f;
    float tB = 1.0f;

    float c1x = control[0].x(), c1y = control[0].y();
    float c2x = control[1].x(), c2y = control[1].y();
    float c3x = control[2].x(), c3y = control[2].y();

    const int offset = pts.size();
    pts.resize(offset + segments);
    PathPoint pp;
    pp.m_x = c1x; pp.m_y = c1y;

    for (int i = 1; i < segments; ++i) {
        tA += step;
        tB -= step;
        const float a = tB * tB;
        const float b = 2.0f * tA * tB;
        const float c = tA * tA;
        pp.m_x = c1x * a + c2x * b + c3x * c;
        pp.m_y = c1y * a + c2y * b + c3y * c;
        pts[offset + i - 1] = pp;
    }

    pp.m_x = c3x; pp.m_y = c3y;
    pts[offset + segments - 1] = pp;
}

static void bezier(int segments, Vector<PathPoint>& pts, const PathPoint* control)
{
    const float step = 1.0f / segments;
    float tA = 0.0f;
    float tB = 1.0f;

    float c1x = control[0].x(), c1y = control[0].y();
    float c2x = control[1].x(), c2y = control[1].y();
    float c3x = control[2].x(), c3y = control[2].y();
    float c4x = control[3].x(), c4y = control[3].y();

    const int offset = pts.size();
    pts.resize(offset + segments);
    PathPoint pp;
    pp.m_x = c1x; pp.m_y = c1y;

    for (int i = 1; i < segments; ++i) {
        tA += step;
        tB -= step;
        const float tAsq = tA * tA;
        const float tBsq = tB * tB;
        const float a = tBsq * tB;
        const float b = 3.0f * tA * tBsq;
        const float c = 3.0f * tB * tAsq;
        const float d = tAsq * tA;
        pp.m_x = c1x * a + c2x * b + c3x * c + c4x * d;
        pp.m_y = c1y * a + c2y * b + c3y * c + c4y * d;
        pts[offset + i - 1] = pp;
    }

    pp.m_x = c4x; pp.m_y = c4y;
    pts[offset + segments - 1] = pp;
}

static bool containsPoint(const FloatRect& r, const FloatPoint& p)
{
    return p.x() >= r.x() && p.y() >= r.y() && p.x() < r.maxX() && p.y() < r.maxY();
}

static void normalizeAngle(float& angle)
{
    angle = fmod(angle, 2 * WTF::piFloat);
    if (angle < 0)
        angle += 2 * WTF::piFloat;
    if (angle < 0.00001f)
        angle = 0;
}

static void transformArcPoint(float& x, float& y, const FloatPoint& c)
{
    x += c.x();
    y += c.y();
}

static void inflateRectToContainPoint(FloatRect& r, float x, float y)
{
    if (r.width() == 0 && r.height() == 0 && r.x() == 0 && r.y() == 0) {
        if (x == 0.f) x = 0.001f;
        if (y == 0.f) y = 0.001f;
        r.setX(x); r.setY(y);
        return;
    }
    if (x < r.x()) { r.setWidth(r.maxX() - x); r.setX(x); }
    else { float w = x - r.x(); if (w > r.width()) r.setWidth(w); }
    if (y < r.y()) { r.setHeight(r.maxY() - y); r.setY(y); }
    else { float h = y - r.y(); if (h > r.height()) r.setHeight(h); }
}

static inline int quadrant(const PathPoint& point, const PathPoint& origin)
{
    return point.m_x < origin.m_x ?
        (point.m_y < origin.m_y ? 2 : 1) :
        (point.m_y < origin.m_y ? 3 : 0);
}

static inline bool isQuadrantOnBottom(int q) { return q == 0 || q == 1; }
static inline int nextQuadrant(int q) { return q == 3 ? 0 : q + 1; }
static inline int quadrantDiff(int q1, int q2)
{
    int d = q1 - q2;
    while (d < 0) d += 4;
    return d;
}

struct PathVector {
    float m_x, m_y;
    PathVector() : m_x(0), m_y(0) {}
    PathVector(float x, float y) : m_x(x), m_y(y) {}
    double angle() const { return atan2(m_y, m_x); }
    operator double() const { return angle(); }
    double length() const { return sqrt(m_x * m_x + m_y * m_y); }
};

static PathVector operator-(const PathPoint& p1, const PathPoint& p2)
{
    return PathVector(p1.m_x - p2.m_x, p1.m_y - p2.m_y);
}

static void addArcPoint(PathPolygon& poly, const PathPoint& center, const PathPoint& radius, double angle)
{
    PathPoint p;
    getEllipsePointByAngle(angle, radius.m_x, radius.m_y, p.m_x, p.m_y);
    transformArcPoint(p.m_x, p.m_y, FloatPoint(center.m_x, center.m_y));
    if (poly.isEmpty() || poly.last() != p)
        poly.append(p);
}

static void addArcPoints(PathPolygon& poly, const PlatformPathElement::ArcTo& data)
{
    const PathPoint& startPoint = poly.last();
    double curAngle = startPoint - data.m_center;
    double endAngle = data.m_end - data.m_center;
    double angleStep = 2. / std::max(data.m_radius.m_x, data.m_radius.m_y);
    if (angleStep > (WTF::piDouble / 8.))
        angleStep = WTF::piDouble / 8.;

    if (data.m_clockwise) {
        if (endAngle <= curAngle || startPoint == data.m_end)
            endAngle += 2 * WTF::piDouble;
    } else {
        angleStep = -angleStep;
        if (endAngle >= curAngle || startPoint == data.m_end)
            endAngle -= 2 * WTF::piDouble;
    }

    for (curAngle += angleStep;
         data.m_clockwise ? curAngle < endAngle : curAngle > endAngle;
         curAngle += angleStep)
        addArcPoint(poly, data.m_center, data.m_radius, curAngle);

    if (poly.isEmpty() || poly.last() != data.m_end)
        poly.append(data.m_end);
}

enum { EFill, EStroke, EClip, EClipOut };

static const int cPointsBufferMaxItems = 1024;

void PlatformPathWKC::drawPolygons(void* dc, const Vector<PathPolygon>& polygons, int type, const AffineTransform* transformation) const
{
    static WKCFloatPoint* gPointsBuffer = new WKCFloatPoint[cPointsBufferMaxItems];

    int total_points = 0;
    for (auto i = polygons.begin(); i != polygons.end(); ++i) {
        int npoints = i->size();
        if (!npoints) continue;
        if (type == EFill || type == EClip || type == EClipOut)
            total_points += npoints + 2;
        else
            total_points += npoints;
    }
    if (!total_points) return;

    WKCFloatPoint* wkcPoints = gPointsBuffer;
    bool allocated = false;
    if (total_points > cPointsBufferMaxItems) {
        wkcPoints = (WKCFloatPoint*)calloc(total_points, sizeof(WKCFloatPoint));
        if (!wkcPoints) return;
        allocated = true;
    } else {
        memset(wkcPoints, 0, sizeof(WKCFloatPoint) * cPointsBufferMaxItems);
    }

    int idx = 0;
    for (auto i = polygons.begin(); i != polygons.end(); ++i) {
        int npoints = i->size();
        if (!npoints) continue;

        if (transformation) {
            for (int i2 = 0; i2 < npoints; ++i2) {
                FloatPoint trPoint = transformation->mapPoint(i->at(i2));
                wkcPoints[idx + i2].fX = trPoint.x();
                wkcPoints[idx + i2].fY = trPoint.y();
            }
        } else {
            for (int i2 = 0; i2 < npoints; ++i2) {
                wkcPoints[idx + i2].fX = i->at(i2).x();
                wkcPoints[idx + i2].fY = i->at(i2).y();
            }
        }

        if (type == EFill || type == EClip || type == EClipOut) {
            if (wkcPoints[idx + npoints - 1].fX != wkcPoints[idx].fX ||
                wkcPoints[idx + npoints - 1].fY != wkcPoints[idx].fY) {
                wkcPoints[idx + npoints].fX = wkcPoints[idx].fX;
                wkcPoints[idx + npoints].fY = wkcPoints[idx].fY;
                ++npoints;
            }
            wkcPoints[idx + npoints].fX = wkcPoints[idx + npoints].fY = FLT_MIN;
            ++npoints;
            idx += npoints;
        } else if (type == EStroke) {
            wkcDrawContextDrawPolylinePeer(dc, npoints, wkcPoints, m_closed, true);
            idx = 0;
        }
    }

    switch (type) {
    case EFill:   wkcDrawContextDrawPolygonPeer(dc, idx, wkcPoints);    break;
    case EStroke: break;
    case EClip:   wkcDrawContextClipPolygonPeer(dc, idx, wkcPoints);    break;
    case EClipOut:wkcDrawContextClipOutPolygonPeer(dc, idx, wkcPoints); break;
    }

    if (allocated)
        free(wkcPoints);
}

int PlatformPathElement::numControlPoints() const
{
    switch (m_type) {
    case PathMoveTo: case PathLineTo: return 1;
    case PathQuadCurveTo: case PathArcTo: return 2;
    case PathBezierCurveTo: return 3;
    default: ASSERT(m_type == PathCloseSubpath); return 0;
    }
}

int PlatformPathElement::numPoints() const
{
    switch (m_type) {
    case PathMoveTo: case PathLineTo: case PathArcTo: return 1;
    case PathQuadCurveTo: return 2;
    case PathBezierCurveTo: return 3;
    default: ASSERT(m_type == PathCloseSubpath); return 0;
    }
}

void PathPolygon::move(const FloatSize& offset)
{
    for (auto i = begin(); i < end(); ++i)
        i->move(offset);
}

void PathPolygon::transform(const AffineTransform& t)
{
    for (auto i = begin(); i < end(); ++i)
        *i = t.mapPoint(*i);
}

bool PathPolygon::contains(const FloatPoint& point) const
{
    if (size() < 3)
        return false;

    int intersected = 0;
    const PathPoint* point1 = &last();
    auto last_ = end();
    int wasNegative = -1;

    for (auto i = begin(); i != last_; ++i) {
        const PathPoint& point2 = *i;
        if (point1->x() != point.x()) {
            if (point2.x() == point.x()) {
                wasNegative = point1->x() < point.x() ? 1 : 0;
            } else if ((point2.x() < point.x()) != (point1->x() < point.x())) {
                float y = (point2.y() - point1->y()) / (point2.x() - point1->x()) * (point.x() - point1->x()) + point1->y();
                if (y >= point.y())
                    ++intersected;
            }
        } else {
            if (point1->y() == point.y())
                return true;
            if (point1->y() > point.y()) {
                if (point2.x() == point.x()) {
                    if (point2.y() <= point.y())
                        return true;
                } else {
                    if (wasNegative < 0) {
                        auto jLast = i;
                        auto j = i;
                        do {
                            if (j == begin()) j = last_;
                            else --j;
                            if (j->x() != point.x()) {
                                wasNegative = j->x() > point.x() ? 0 : 1;
                                break;
                            }
                        } while (j != jLast);
                        if (wasNegative < 0)
                            return false;
                    }
                    if (wasNegative ? point2.x() > point.x() : point2.x() < point.x())
                        ++intersected;
                }
            } else if (point2.x() == point.x() && point2.y() >= point.y())
                return true;
        }
        point1 = &point2;
    }
    return intersected & 1;
}

void PlatformPathElement::move(const FloatSize& offset)
{
    int n = numControlPoints();
    for (int i = 0; i < n; ++i)
        m_data.m_points[i].move(offset);
}

void PlatformPathElement::transform(const AffineTransform& t)
{
    int n = numControlPoints();
    for (int i = 0; i < n; ++i) {
        FloatPoint p = t.mapPoint(m_data.m_points[i]);
        m_data.m_points[i].set(p.x(), p.y());
    }
}

void PlatformPathElement::inflateRectToContainMe(FloatRect& r, const FloatPoint& lastPoint) const
{
    if (m_type == PathArcTo) {
        const ArcTo& data = m_data.m_arcToData;
        PathPoint startPoint;
        startPoint = lastPoint;
        PathPoint endPoint = data.m_end;
        if (!data.m_clockwise)
            std::swap(startPoint, endPoint);

        int q0 = quadrant(startPoint, data.m_center);
        int q1 = quadrant(endPoint, data.m_center);
        bool containsExtremes[4] = { false, false, false, false };
        static const PathPoint extremeVectors[4] = { {0,1}, {-1,0}, {0,-1}, {1,0} };

        if (q0 == q1) {
            if (startPoint.m_x == endPoint.m_x ||
                isQuadrantOnBottom(q0) != (startPoint.m_x > endPoint.m_x))
                for (int i = 0; i < 4; ++i)
                    containsExtremes[i] = true;
        } else {
            int extreme = q0;
            int diff = quadrantDiff(q1, q0);
            for (int i = 0; i < diff; ++i) {
                containsExtremes[extreme] = true;
                extreme = nextQuadrant(extreme);
            }
        }

        inflateRectToContainPoint(r, startPoint.m_x, startPoint.m_y);
        inflateRectToContainPoint(r, endPoint.m_x, endPoint.m_y);
        for (int i = 0; i < 4; ++i)
            if (containsExtremes[i])
                inflateRectToContainPoint(r,
                    data.m_center.m_x + data.m_radius.m_x * extremeVectors[i].m_x,
                    data.m_center.m_y + data.m_radius.m_y * extremeVectors[i].m_y);
    } else {
        int n = numPoints();
        for (int i = 0; i < n; ++i)
            inflateRectToContainPoint(r, m_data.m_points[i].m_x, m_data.m_points[i].m_y);
    }
}

PlatformPathElement::PlaformPathElementType PlatformPathElement::type() const
{
    return m_type;
}

PlatformPathWKC::PlatformPathWKC()
    : m_penLifted(true)
    , m_closed(false)
{
    m_currentPoint.clear();
}

PlatformPathWKC::PlatformPathWKC(const PlatformPathWKC* obj)
    : m_elements(obj->m_elements)
    , m_boundingRect(obj->m_boundingRect)
    , m_subpaths(obj->m_subpaths)
    , m_currentPoint(obj->m_currentPoint)
    , m_penLifted(obj->m_penLifted)
    , m_closed(obj->m_closed)
{
}

PlatformPathWKC::~PlatformPathWKC()
{
}

void PlatformPathWKC::ensureSubpath()
{
    if (m_penLifted) {
        m_penLifted = false;
        m_subpaths.append(PathPolygon());
        m_subpaths.last().append(m_currentPoint);
    } else
        ASSERT(!m_subpaths.isEmpty());
}

void PlatformPathWKC::addToSubpath(const PlatformPathElement& e)
{
    if (e.platformType() == PlatformPathElement::PathMoveTo) {
        m_penLifted = true;
        m_currentPoint = e.pointAt(0);
    } else if (e.platformType() == PlatformPathElement::PathCloseSubpath) {
        m_penLifted = true;
        if (!m_subpaths.isEmpty()) {
            if (m_currentPoint != m_subpaths.last()[0]) {
                m_subpaths.last().append(m_subpaths.last()[0]);
                m_currentPoint = m_subpaths.last()[0];
            }
        } else
            m_currentPoint.clear();
    } else {
        ensureSubpath();
        switch (e.platformType()) {
        case PlatformPathElement::PathLineTo:
            m_subpaths.last().append(e.pointAt(0));
            break;
        case PlatformPathElement::PathArcTo:
            addArcPoints(m_subpaths.last(), e.arcTo());
            break;
        case PlatformPathElement::PathQuadCurveTo: {
            PathPoint control[] = { m_currentPoint, e.pointAt(0), e.pointAt(1) };
            quadCurve(50, m_subpaths.last(), control);
            break;
        }
        case PlatformPathElement::PathBezierCurveTo: {
            PathPoint control[] = { m_currentPoint, e.pointAt(0), e.pointAt(1), e.pointAt(2) };
            bezier(100, m_subpaths.last(), control);
            break;
        }
        default:
            ASSERT_NOT_REACHED();
            break;
        }
        m_currentPoint = m_subpaths.last().last();
    }
}

void PlatformPathWKC::append(const PlatformPathElement& e)
{
    e.inflateRectToContainMe(m_boundingRect, lastPoint());
    addToSubpath(e);
    m_elements.append(e);
}

void PlatformPathWKC::append(const PlatformPathWKC& p)
{
    const PlatformPathElements& e = p.elements();
    for (auto it = e.begin(); it != e.end(); ++it) {
        addToSubpath(*it);
        it->inflateRectToContainMe(m_boundingRect, lastPoint());
        m_elements.append(*it);
    }
}

void PlatformPathWKC::clear()
{
    m_elements.clear();
    m_boundingRect = FloatRect();
    m_subpaths.clear();
    m_currentPoint.clear();
    m_penLifted = true;
    m_closed = false;
}

void PlatformPathWKC::strokePath(void* dc, const AffineTransform* transformation) const
{
    drawPolygons(dc, m_subpaths, EStroke, transformation);
}

void PlatformPathWKC::fillPath(void* dc, const AffineTransform* transformation) const
{
    drawPolygons(dc, m_subpaths, EFill, transformation);
}

void PlatformPathWKC::clipPath(void* dc, const AffineTransform* transformation) const
{
    drawPolygons(dc, m_subpaths, EClip, transformation);
}

void PlatformPathWKC::clipOutPath(void* dc, const AffineTransform* transformation) const
{
    drawPolygons(dc, m_subpaths, EClipOut, transformation);
}

void PlatformPathWKC::translate(const FloatSize& size)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it)
        it->move(size);
    m_boundingRect.move(size);
    for (auto it = m_subpaths.begin(); it != m_subpaths.end(); ++it)
        it->move(size);
}

void PlatformPathWKC::transform(const AffineTransform& t)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it)
        it->transform(t);
    m_boundingRect = t.mapRect(m_boundingRect);
    for (auto it = m_subpaths.begin(); it != m_subpaths.end(); ++it)
        it->transform(t);
}

bool PlatformPathWKC::contains(const FloatPoint& point, WindRule rule) const
{
    if (!containsPoint(m_boundingRect, point))
        return false;
    for (auto i = m_subpaths.begin(); i != m_subpaths.end(); ++i)
        if (i->contains(point))
            return true;
    return false;
}

void PlatformPathWKC::moveTo(const FloatPoint& point)
{
    PlatformPathElement::MoveTo data = { { point.x(), point.y() } };
    append(PlatformPathElement(data));
}

void PlatformPathWKC::addLineTo(const FloatPoint& point)
{
    PlatformPathElement::LineTo data = { { point.x(), point.y() } };
    append(PlatformPathElement(data));
}

void PlatformPathWKC::addQuadCurveTo(const FloatPoint& cp, const FloatPoint& p)
{
    PlatformPathElement::QuadCurveTo data = { { cp.x(), cp.y() }, { p.x(), p.y() } };
    append(PlatformPathElement(data));
}

void PlatformPathWKC::addBezierCurveTo(const FloatPoint& cp1, const FloatPoint& cp2, const FloatPoint& p)
{
    PlatformPathElement::BezierCurveTo data = { { cp1.x(), cp1.y() }, { cp2.x(), cp2.y() }, { p.x(), p.y() } };
    append(PlatformPathElement(data));
}

void PlatformPathWKC::addArcTo(const FloatPoint& fp1, const FloatPoint& fp2, float radius)
{
    const PathPoint& p0 = m_currentPoint;
    PathPoint p1; p1 = fp1;
    PathPoint p2; p2 = fp2;

    if (!radius || p0 == p1 || p1 == p2) {
        addLineTo(p1);
        return;
    }

    PathVector v01 = p0 - p1;
    PathVector v21 = p2 - p1;
    double cross = v01.m_x * v21.m_y - v01.m_y * v21.m_x;

    if (fabs(cross) < 1E-10) {
        addLineTo(p1);
        return;
    }

    double d01 = v01.length();
    double d21 = v21.length();
    double angle = (WTF::piDouble - fabs(asin(cross / (d01 * d21)))) * 0.5;
    double span = radius * tan(angle);

    double rate = span / d01;
    PathPoint startPoint;
    startPoint.m_x = p1.m_x + v01.m_x * rate;
    startPoint.m_y = p1.m_y + v01.m_y * rate;
    addLineTo(startPoint);

    rate = span / d21;
    PathPoint endPoint;
    endPoint.m_x = p1.m_x + v21.m_x * rate;
    endPoint.m_y = p1.m_y + v21.m_y * rate;

    PathPoint midPoint;
    midPoint.m_x = (startPoint.m_x + endPoint.m_x) * 0.5f;
    midPoint.m_y = (startPoint.m_y + endPoint.m_y) * 0.5f;

    PathVector vm1 = midPoint - p1;
    double dm1 = vm1.length();
    double d = sqrt(radius * radius + span * span);

    rate = d / dm1;
    PathPoint centerPoint;
    centerPoint.m_x = p1.m_x + vm1.m_x * rate;
    centerPoint.m_y = p1.m_y + vm1.m_y * rate;

    PlatformPathElement::ArcTo data = { endPoint, centerPoint, { radius, radius }, cross < 0 };
    append(PlatformPathElement(data));
}

void PlatformPathWKC::closeSubpath()
{
    append(PlatformPathElement());
    m_closed = true;
}

void PlatformPathWKC::addEllipse(const FloatPoint& p, float a, float b, float sar, float ear, bool anticlockwise)
{
    float startX, startY, endX, endY;
    normalizeAngle(sar);
    normalizeAngle(ear);
    getEllipsePointByAngle(sar, a, b, startX, startY);
    getEllipsePointByAngle(ear, a, b, endX, endY);
    transformArcPoint(startX, startY, p);
    transformArcPoint(endX, endY, p);

    FloatPoint start(startX, startY);
    if (!m_subpaths.isEmpty())
        addLineTo(start);
    else
        moveTo(start);

    PlatformPathElement::ArcTo data = { {endX, endY}, {p.x(), p.y()}, {a, b}, !anticlockwise };
    append(PlatformPathElement(data));
    m_closed = false;
}

void PlatformPathWKC::addRect(const FloatRect& r)
{
    moveTo(r.location());
    float right = r.maxX(), bottom = r.maxY();
    addLineTo(FloatPoint(right, r.y()));
    addLineTo(FloatPoint(right, bottom));
    addLineTo(FloatPoint(r.x(), bottom));
    addLineTo(r.location());
    m_closed = true;
}

void PlatformPathWKC::addEllipse(const FloatRect& r)
{
    FloatSize radius(r.width() * 0.5f, r.height() * 0.5f);
    addEllipse(r.location() + radius, radius.width(), radius.height(), 0, 0, false);
}

FloatRect PlatformPathWKC::boundingRectWithStyle(const GraphicsContext* dc) const
{
    const float w = dc->strokeThickness();
    FloatRect r = m_boundingRect;
    r.inflate(w * 1.0f);
    return r;
}

bool PlatformPathWKC::strokeContains(const GraphicsContext* dc, const FloatPoint& point)
{
    const float w = dc->strokeThickness();
    FloatRect outside = m_boundingRect;
    outside.inflate(w * 0.5f);
    if (!containsPoint(outside, point))
        return false;
    FloatRect inside = m_boundingRect;
    inside.inflate(-w * 0.5f);
    if (containsPoint(inside, point))
        return false;
    return true;
}

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
