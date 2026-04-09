/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
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

#if !USE(WKC_CAIRO)

#include "GraphicsContext.h"
#include "AffineTransform.h"
#include "Color.h"
#include "FloatQuad.h"
#include "FloatRect.h"
#include "FloatRoundedRect.h"
#include "FloatSegment.h"
#include "Gradient.h"
#include "ImageBuffer.h"
#include "ImageWKC.h"
#include "IntRect.h"
#include "NativeImage.h"
#include "NotImplemented.h"
#include "Path.h"
#include "Pattern.h"
#include "PlatformPathWKC.h"
#include <wtf/MathExtras.h>

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

// ---------------------------------------------------------------------------
// Platform private data
// ---------------------------------------------------------------------------

class GraphicsContextPlatformPrivateData {
public:
    GraphicsContextPlatformPrivateData()
        : m_opacity(1.0f)
        , m_clip_context(C_INVALID)
    { }

    float m_opacity;
    enum m_context_type {
        C_INVALID   = 0,
        C_UNKNOWN   = 1,
        C_CANVAS_2D = 2,
    };
    enum m_context_type m_clip_context;
    Path m_clip;
    AffineTransform m_clip_transform;
    AffineTransform m_transform;
    AffineTransform m_itransform;
};

class GraphicsContextPlatformPrivate : public GraphicsContextPlatformPrivateData {
    WTF_MAKE_TZONE_ALLOCATED_INLINE(GraphicsContextPlatformPrivate);
public:
    GraphicsContextPlatformPrivate() : m_drawcontext(nullptr) { }

    ~GraphicsContextPlatformPrivate()
    {
        while (!m_backupData.isEmpty())
            restore(false);
    }

    void save()
    {
        if (m_drawcontext)
            wkcDrawContextSaveStatePeer(m_drawcontext);
        m_backupData.append(*static_cast<GraphicsContextPlatformPrivateData*>(this));
    }

    void restore(bool restoreclip)
    {
        if (m_backupData.isEmpty()) return;
        if (m_drawcontext)
            wkcDrawContextRestoreStatePeer(m_drawcontext);
        GraphicsContextPlatformPrivateData::operator=(m_backupData.last());
        m_backupData.removeLast();
        if (restoreclip && m_drawcontext) {
            auto* pp = static_cast<PlatformPathWKC*>(m_clip.platformPath());
            if (C_CANVAS_2D == m_clip_context) {
                wkcDrawContextClearClipPolygonPeer(m_drawcontext);
                wkcDrawContextCanvasClipPathBeginPeer(m_drawcontext);
            } else {
                wkcDrawContextClearClipPolygonPeer(m_drawcontext);
            }
            if (pp) {
                pp->clipPath(m_drawcontext, &m_clip_transform);
                if (C_CANVAS_2D == m_clip_context)
                    wkcDrawContextCanvasClipPathEndPeer(m_drawcontext);
            }
        }
    }

    bool mapRect(const FloatRect& rect, WKCFloatPoint* p)
    {
        FloatQuad q = m_transform.mapQuad(rect);
        FloatRect br = q.boundingBox();
        if (q.isEmpty()) return false;
        if (br.width() < 1.f || br.height() < 1.f) return false;

        p[0].fX = q.p1().x(); p[0].fY = q.p1().y();
        p[1].fX = q.p2().x(); p[1].fY = q.p2().y();
        p[2].fX = q.p3().x(); p[2].fY = q.p3().y();
        p[3].fX = q.p4().x(); p[3].fY = q.p4().y();

        int d = (p[1].fX-p[0].fX)*(p[2].fY-p[0].fY) - (p[2].fX-p[0].fX)*(p[1].fY-p[0].fY);
        if (!d) return false;
        d = (p[3].fX-p[0].fX)*(p[2].fY-p[0].fY) - (p[2].fX-p[0].fX)*(p[3].fY-p[0].fY);
        if (!d) return false;
        return true;
    }

    void* m_drawcontext;
    Vector<GraphicsContextPlatformPrivateData> m_backupData;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static inline unsigned int platformColor(const Color& color)
{
    auto s = color.toColorTypeLossy<SRGBA<uint8_t>>();
    return ((unsigned int)s.alpha << 24)
         | ((unsigned int)s.red   << 16)
         | ((unsigned int)s.green <<  8)
         |  (unsigned int)s.blue;
}

static inline bool colorIsVisible(const Color& color)
{
    return color.toColorTypeLossy<SRGBA<uint8_t>>().alpha != 0;
}

static inline int platformStyle(StrokeStyle style)
{
    switch (style) {
    case StrokeStyle::NoStroke:     return WKC_STROKESTYLE_NO;
    case StrokeStyle::SolidStroke:  return WKC_STROKESTYLE_SOLID;
    case StrokeStyle::DottedStroke: return WKC_STROKESTYLE_DOTTED;
    case StrokeStyle::DashedStroke: return WKC_STROKESTYLE_DASHED;
    default: return WKC_STROKESTYLE_SOLID;
    }
}

static inline int platformTextDrawingMode(TextDrawingModeFlags mode)
{
    int ret = WKC_FONT_DRAWINGMODE_INVISIBLE;
    if (mode.contains(TextDrawingMode::Fill))   ret |= WKC_FONT_DRAWINGMODE_FILL;
    if (mode.contains(TextDrawingMode::Stroke)) ret |= WKC_FONT_DRAWINGMODE_STROKE;
    return ret;
}

static inline void platformPoint(const FloatPoint& in, WKCFloatPoint& out)
{
    out.fX = in.x();
    out.fY = in.y();
}

// WKCFloatRect has nested fPoint / fSize members
static inline void platformRect(const FloatRect& in, WKCFloatRect& out)
{
    out.fPoint.fX    = in.x();
    out.fPoint.fY    = in.y();
    out.fSize.fWidth  = in.width();
    out.fSize.fHeight = in.height();
}

static inline void platformRect(const IntRect& in, WKCFloatRect& out)
{
    out.fPoint.fX    = in.x();
    out.fPoint.fY    = in.y();
    out.fSize.fWidth  = in.width();
    out.fSize.fHeight = in.height();
}

static void applyMatrix(void* dc, GraphicsContextPlatformPrivate* d)
{
    if (!dc) return;
    d->m_itransform = d->m_transform.inverse().value_or(AffineTransform());
    wkcDrawContextSetMatrixPeer(dc,
        d->m_transform.a(), d->m_transform.b(),
        d->m_transform.c(), d->m_transform.d(),
        d->m_transform.e(), d->m_transform.f());
    wkcDrawContextSetInvertMatrixPeer(dc,
        d->m_itransform.a(), d->m_itransform.b(),
        d->m_itransform.c(), d->m_itransform.d(),
        d->m_itransform.e(), d->m_itransform.f());
}

// ---------------------------------------------------------------------------
// GraphicsContextWKC — concrete subclass of GraphicsContext
// ---------------------------------------------------------------------------

class GraphicsContextWKC final : public GraphicsContext {
    WTF_MAKE_TZONE_ALLOCATED_INLINE(GraphicsContextWKC);
public:
    explicit GraphicsContextWKC(void* drawContext)
        : m_data(new GraphicsContextPlatformPrivate())
    {
        m_data->m_drawcontext = drawContext;
        if (drawContext) {
            wkcDrawContextSetFillColorPeer(drawContext, platformColor(fillColor()));
            wkcDrawContextSetStrokeColorPeer(drawContext, platformColor(strokeColor()));
        }
    }

    ~GraphicsContextWKC() override
    {
        if (m_data && m_data->m_drawcontext)
            wkcDrawContextFlushPeer(m_data->m_drawcontext);
        delete m_data;
    }

    PlatformGraphicsContext* platformContext() const override
    {
        return static_cast<PlatformGraphicsContext*>(m_data->m_drawcontext);
    }

    RenderingMode renderingMode() const override { return RenderingMode::Unaccelerated; }

    // --- State ---

    void save(GraphicsContextState::Purpose purpose = GraphicsContextState::Purpose::SaveRestore) override
    {
        GraphicsContext::save(purpose);
        m_data->save();
    }

    void restore(GraphicsContextState::Purpose purpose = GraphicsContextState::Purpose::SaveRestore) override
    {
        GraphicsContext::restore(purpose);
        m_data->restore(true);
    }

    void didUpdateState(GraphicsContextState& state) override
    {
        if (paintingDisabled()) return;
        auto changes = state.changes();

        // Use GraphicsContext accessors directly — GraphicsContextState doesn't
        // expose colour getters directly after the changes() refactor.
        if (changes.contains(GraphicsContextState::Change::StrokeThickness))
            wkcDrawContextSetStrokeThicknessPeer(m_data->m_drawcontext, strokeThickness());
        if (changes.contains(GraphicsContextState::Change::StrokeStyle))
            wkcDrawContextSetStrokeStylePeer(m_data->m_drawcontext, platformStyle(strokeStyle()));
        if (changes.contains(GraphicsContextState::Change::TextDrawingMode))
            wkcDrawContextSetTextDrawingModePeer(m_data->m_drawcontext, platformTextDrawingMode(textDrawingMode()));
        if (changes.contains(GraphicsContextState::Change::ShouldAntialias))
            wkcDrawContextSetShouldAntialiasPeer(m_data->m_drawcontext, shouldAntialias() ? 1 : 0);
        if (changes.contains(GraphicsContextState::Change::CompositeMode)) {
            int ope = WKC_COMPOSITEOPERATION_SOURCEOVER;
            switch (compositeOperation()) {
            case CompositeOperator::Clear:           ope = WKC_COMPOSITEOPERATION_CLEAR; break;
            case CompositeOperator::Copy:            ope = WKC_COMPOSITEOPERATION_COPY; break;
            case CompositeOperator::SourceOver:      ope = WKC_COMPOSITEOPERATION_SOURCEOVER; break;
            case CompositeOperator::SourceIn:        ope = WKC_COMPOSITEOPERATION_SOURCEIN; break;
            case CompositeOperator::SourceOut:       ope = WKC_COMPOSITEOPERATION_SOURCEOUT; break;
            case CompositeOperator::SourceAtop:      ope = WKC_COMPOSITEOPERATION_SOURCEATOP; break;
            case CompositeOperator::DestinationOver: ope = WKC_COMPOSITEOPERATION_DESTINATIONOVER; break;
            case CompositeOperator::DestinationIn:   ope = WKC_COMPOSITEOPERATION_DESTINATIONIN; break;
            case CompositeOperator::DestinationOut:  ope = WKC_COMPOSITEOPERATION_DESTINATIONOUT; break;
            case CompositeOperator::DestinationAtop: ope = WKC_COMPOSITEOPERATION_DESTINATIONATOP; break;
            case CompositeOperator::XOR:             ope = WKC_COMPOSITEOPERATION_XOR; break;
            case CompositeOperator::PlusDarker:      ope = WKC_COMPOSITEOPERATION_PLUSDARKER; break;
            case CompositeOperator::PlusLighter:     ope = WKC_COMPOSITEOPERATION_PLUSLIGHTER; break;
            default: break;
            }
            wkcDrawContextSetCompositeOperationPeer(m_data->m_drawcontext, ope);
        }
        if (changes.contains(GraphicsContextState::Change::DropShadow)) {
            auto shadow = dropShadow();
            if (shadow.has_value()) {
                WKCFloatSize ws = { shadow->offset.width(), shadow->offset.height() };
                wkcDrawContextSetShadowPeer(m_data->m_drawcontext, 0, nullptr, nullptr, nullptr,
                    &ws, shadow->blurRadius, platformColor(shadow->color), false);
            } else {
                wkcDrawContextClearShadowPeer(m_data->m_drawcontext);
            }
        }
        if (changes.contains(GraphicsContextState::Change::Alpha)) {
            float a = alpha();
            m_data->m_opacity = a;
            wkcDrawContextSetAlphaPeer(m_data->m_drawcontext, static_cast<int>(255.f * a));
        }
        // Sync stroke/fill colours on every state update
        wkcDrawContextSetStrokeColorPeer(m_data->m_drawcontext, platformColor(strokeColor()));
        wkcDrawContextSetFillColorPeer(m_data->m_drawcontext, platformColor(fillColor()));
    }

    // --- Transform ---

    void translate(float tx, float ty) override
    {
        m_data->m_transform.translate(tx, ty);
        applyMatrix(m_data->m_drawcontext, m_data);
    }

    void rotate(float angle) override
    {
        m_data->m_transform.rotate(rad2deg(angle));
        applyMatrix(m_data->m_drawcontext, m_data);
    }

    void scale(const FloatSize& s) override
    {
        m_data->m_transform.scaleNonUniform(s.width(), s.height());
        applyMatrix(m_data->m_drawcontext, m_data);
    }

    void concatCTM(const AffineTransform& t) override
    {
        m_data->m_transform *= t;
        applyMatrix(m_data->m_drawcontext, m_data);
    }

    void setCTM(const AffineTransform& m) override
    {
        m_data->m_transform = m;
        applyMatrix(m_data->m_drawcontext, m_data);
    }

    AffineTransform getCTM(IncludeDeviceScale = DefinitelyIncludeDeviceScale) const override
    {
        return m_data->m_transform;
    }

    // --- Basic drawing ---

    void drawRect(const FloatRect& rect, float = 1) override
    {
        if (paintingDisabled()) return;
        WKCFloatRect r; platformRect(rect, r);
        wkcDrawContextDrawRectPeer(m_data->m_drawcontext, &r);
    }

    void drawLine(const FloatPoint& p1, const FloatPoint& p2) override
    {
        if (paintingDisabled()) return;
        WKCFloatPoint wp1, wp2;
        platformPoint(p1, wp1); platformPoint(p2, wp2);
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
        wkcDrawContextDrawLinePeer(m_data->m_drawcontext, &wp1, &wp2);
    }

    void drawEllipse(const FloatRect& rect) override
    {
        if (paintingDisabled()) return;
        WKCFloatRect r; platformRect(rect, r);
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
        wkcDrawContextDrawEllipsePeer(m_data->m_drawcontext, &r);
    }

    void strokeArc(const PathArc& arc) override
    {
        if (paintingDisabled()) return;
        FloatRect rect(arc.center.x() - arc.radius, arc.center.y() - arc.radius, arc.radius * 2, arc.radius * 2);
        WKCFloatRect r; platformRect(rect, r);
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
        wkcDrawContextStrokeArcPeer(m_data->m_drawcontext, &r,
            static_cast<int>(rad2deg(arc.startAngle)),
            static_cast<int>(rad2deg(arc.endAngle - arc.startAngle)));
    }

    // --- Fill ---

    void fillRect(const FloatRect& rect, RequiresClipToRect = RequiresClipToRect::Yes) override
    {
        if (paintingDisabled()) return;
        m_data->save();
        WKCPeerPattern* pt = nullptr;
        AffineTransform affine;
        WKCFloatPoint p[6];

        if (auto* pattern = fillPattern())
            pt = reinterpret_cast<WKCPeerPattern*>(pattern->createPlatformPattern(affine));
        // Gradient: not supported in WKC peer without platformGradient() — skip
        else if (!colorIsVisible(fillColor()) || !m_data->m_opacity) {
            m_data->restore(false); return;
        }

        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);
        if (m_data->mapRect(rect, p)) {
            WKCFloatPoint_SetPoint(&p[4], &p[0]);
            WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);
            wkcDrawContextDrawPolygonPeer(m_data->m_drawcontext, 6, p);
        }
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, nullptr);
        m_data->restore(false);
    }

    void fillRect(const FloatRect& rect, const Color& color) override
    {
        if (paintingDisabled()) return;
        m_data->save();
        int type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);

        if (type == WKC_OFFSCREEN_TYPE_IMAGEBUF) {
            WKCFloatPoint p[6];
            if (m_data->mapRect(rect, p)) {
                wkcDrawContextSetFillColorPeer(m_data->m_drawcontext, platformColor(color));
                WKCFloatPoint_SetPoint(&p[4], &p[0]);
                WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);
                wkcDrawContextDrawPolygonPeer(m_data->m_drawcontext, 6, p);
            }
        } else {
            auto shadow = dropShadow();
            if (shadow.has_value()) {
                WKCFloatSize ws = { shadow->offset.width(), shadow->offset.height() };
                if (!shadow->blurRadius) {
                    WKCFloatRect r; platformRect(rect, r);
                    r.fPoint.fX += shadow->offset.width();
                    r.fPoint.fY += shadow->offset.height();
                    wkcDrawContextSetStrokeStylePeer(m_data->m_drawcontext, platformStyle(StrokeStyle::NoStroke));
                    wkcDrawContextFillRectPeer(m_data->m_drawcontext, &r, platformColor(shadow->color));
                } else {
                    wkcDrawContextSetShadowPeer(m_data->m_drawcontext, 0, nullptr, nullptr, nullptr,
                        &ws, shadow->blurRadius, platformColor(shadow->color), false);
                }
            }
            WKCFloatRect r; platformRect(rect, r);
            wkcDrawContextSetStrokeStylePeer(m_data->m_drawcontext, platformStyle(StrokeStyle::NoStroke));
            wkcDrawContextFillRectPeer(m_data->m_drawcontext, &r, platformColor(color));
            if (shadow.has_value())
                wkcDrawContextClearShadowPeer(m_data->m_drawcontext);
        }
        m_data->restore(false);
    }

    void fillRect(const FloatRect& rect, const Color& color, CompositeOperator op, BlendMode blend = BlendMode::Normal) override
    {
        setCompositeOperation(op, blend);
        fillRect(rect, color);
    }

    void fillRect(const FloatRect& rect, Gradient&, const AffineTransform&, RequiresClipToRect = RequiresClipToRect::Yes) override
    {
        notImplemented(); // WKC gradient peer not wired to modern Gradient API
    }

    void fillRect(const FloatRect& rect, Gradient& g) override
    {
        fillRect(rect, g, AffineTransform());
    }

    void fillRoundedRectImpl(const FloatRoundedRect& roundedRect, const Color& color) override
    {
        if (paintingDisabled()) return;
        m_data->save();
        Path path;
        path.addRoundedRect(roundedRect);
        wkcDrawContextSetFillColorPeer(m_data->m_drawcontext, platformColor(color));
        fillPath(path);
        m_data->restore(false);
    }

    void fillRectWithRoundedHole(const FloatRect& rect, const FloatRoundedRect& roundedHoleRect, const Color& color) override
    {
        if (paintingDisabled()) return;
        Path path;
        path.addRect(rect);
        if (!roundedHoleRect.radii().isZero())
            path.addRoundedRect(roundedHoleRect);
        else
            path.addRect(roundedHoleRect.rect());
        WindRule oldRule = fillRule();
        Color oldColor = fillColor();
        setFillRule(WindRule::EvenOdd);
        setFillColor(color);
        fillPath(path);
        setFillRule(oldRule);
        setFillColor(oldColor);
    }

    void fillPath(const Path& path) override
    {
        if (paintingDisabled() || !m_data->m_opacity) return;

        WKCPeerPattern* pt = nullptr;
        AffineTransform affine;
        if (auto* pattern = fillPattern())
            pt = reinterpret_cast<WKCPeerPattern*>(pattern->createPlatformPattern(affine));
        else if (fillGradient()) { notImplemented(); return; }
        else if (!colorIsVisible(fillColor())) return;

        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);
        wkcDrawContextSetDrawAccuratePeer(m_data->m_drawcontext, true);
        int savedFillRule = wkcDrawContextGetFillRulePeer(m_data->m_drawcontext);
        wkcDrawContextSetFillRulePeer(m_data->m_drawcontext,
            (fillRule() == WindRule::EvenOdd) ? WKC_FILLRULE_EVENODD : WKC_FILLRULE_WINDING);

        m_data->save();
        auto* pp = static_cast<PlatformPathWKC*>(path.platformPath());
        if (pp) pp->fillPath(m_data->m_drawcontext, &m_data->m_transform);
        m_data->restore(false);

        wkcDrawContextSetFillRulePeer(m_data->m_drawcontext, savedFillRule);
        wkcDrawContextSetDrawAccuratePeer(m_data->m_drawcontext, false);
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, nullptr);
    }

    void strokePath(const Path& path) override
    {
        if (paintingDisabled() || !m_data->m_opacity) return;

        WKCPeerPattern* pt = nullptr;
        AffineTransform affine;
        if (auto* pattern = strokePattern())
            pt = reinterpret_cast<WKCPeerPattern*>(pattern->createPlatformPattern(affine));
        else if (strokeGradient()) { notImplemented(); return; }
        else if (!colorIsVisible(strokeColor())) return;

        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);
        wkcDrawContextSetDrawAccuratePeer(m_data->m_drawcontext, true);
        m_data->save();
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
        auto* pp = static_cast<PlatformPathWKC*>(path.platformPath());
        if (pp) pp->strokePath(m_data->m_drawcontext, &m_data->m_transform);
        m_data->restore(false);
        wkcDrawContextSetDrawAccuratePeer(m_data->m_drawcontext, false);
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, nullptr);
    }

    void strokeRect(const FloatRect& rect, float) override
    {
        if (paintingDisabled()) return;
        int type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);
        m_data->save();
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
        if (type == WKC_OFFSCREEN_TYPE_IMAGEBUF) {
            WKCFloatPoint p[5];
            FloatRect fr(rect);
            bool drawjoin = true;
            if (!fr.width() && fr.height())      { fr.setWidth(1);  drawjoin = false; }
            else if (!fr.height() && fr.width()) { fr.setHeight(1); drawjoin = false; }
            if (m_data->mapRect(fr, p)) {
                WKCFloatPoint_SetPoint(&p[4], &p[0]);
                wkcDrawContextDrawPolylinePeer(m_data->m_drawcontext, 5, p, true, drawjoin);
            }
        } else {
            WKCFloatRect r; platformRect(rect, r);
            wkcDrawContextStrokeRectPeer(m_data->m_drawcontext, &r);
        }
        m_data->restore(false);
    }

    void clearRect(const FloatRect& rect) override
    {
        if (paintingDisabled()) return;
        WKCFloatPoint p[6];
        if (m_data->mapRect(rect, p)) {
            WKCFloatPoint_SetPoint(&p[4], &p[0]);
            WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);
            wkcDrawContextClearPolygonPeer(m_data->m_drawcontext, 6, p);
        }
    }

    // --- Clipping ---

    void clip(const FloatRect& rect) override
    {
        if (paintingDisabled()) return;
        WKCFloatRect r; platformRect(rect, r);
        wkcDrawContextClipPeer(m_data->m_drawcontext, &r);
    }

    void clipOut(const FloatRect& rect) override
    {
        if (paintingDisabled()) return;
        WKCFloatRect r; platformRect(rect, r);
        wkcDrawContextClipOutRectPeer(m_data->m_drawcontext, &r);
    }

    void clipOut(const Path& path) override
    {
        if (paintingDisabled() || path.isEmpty()) return;
        auto* pp = static_cast<PlatformPathWKC*>(path.platformPath());
        if (pp) pp->clipOutPath(m_data->m_drawcontext, &m_data->m_transform);
    }

    void clipPath(const Path& path, WindRule = WindRule::EvenOdd) override
    {
        if (paintingDisabled()) return;
        if (path.isEmpty()) { clip(FloatRect()); return; }
        m_data->m_clip_context = GraphicsContextPlatformPrivateData::C_UNKNOWN;
        m_data->m_clip = path;
        m_data->m_clip_transform = m_data->m_transform;
        auto* pp = static_cast<PlatformPathWKC*>(path.platformPath());
        wkcDrawContextClearClipPolygonPeer(m_data->m_drawcontext);
        if (pp) pp->clipPath(m_data->m_drawcontext, &m_data->m_transform);
    }

    void resetClip() override
    {
        if (paintingDisabled()) return;
        wkcDrawContextClearClipPolygonPeer(m_data->m_drawcontext);
    }

    void clipToImageBuffer(ImageBuffer&, const FloatRect&) override { notImplemented(); }

    // --- Transparency layers ---

    void beginTransparencyLayer(float opacity) override
    {
        GraphicsContext::beginTransparencyLayer(opacity);
        m_data->save();
        m_data->m_opacity *= opacity;
        wkcDrawContextBeginTransparencyLayerPeer(m_data->m_drawcontext,
            static_cast<unsigned char>(m_data->m_opacity * 255.f));
    }

    void endTransparencyLayer() override
    {
        GraphicsContext::endTransparencyLayer();
        wkcDrawContextEndTransparencyLayerPeer(m_data->m_drawcontext);
        m_data->restore(false);
    }

    // --- Focus ring ---

    void drawFocusRing(const Path& path, float outlineWidth, const Color& color, float) override
    {
        if (paintingDisabled()) return;
        auto* pp = static_cast<PlatformPathWKC*>(path.platformPath());
        if (!pp) return;
        m_data->save();
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(color), outlineWidth, WKC_STROKESTYLE_DOTTED);
        pp->strokePath(m_data->m_drawcontext, &m_data->m_transform);
        m_data->restore(false);
    }

    void drawFocusRing(const Vector<FloatRect>& rects, float outlineWidth, const Color& color, float) override
    {
        if (paintingDisabled() || rects.isEmpty()) return;
        m_data->save();
        wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(color), outlineWidth, WKC_STROKESTYLE_SOLID);
        wkcDrawContextSetFillColorPeer(m_data->m_drawcontext, 0);
        FloatRect fr;
        for (auto& r : rects) fr.unite(r);
        fr.inflate(outlineWidth);
        WKCFloatRect r = {}; platformRect(fr, r);
        wkcDrawContextDrawRectPeer(m_data->m_drawcontext, &r);
        m_data->restore(false);
    }

    // --- Text underline ---

    void drawLinesForText(const FloatPoint& origin, float /*thickness*/, std::span<const FloatSegment> lineSegments, bool isPrinting, bool /*doubleLines*/, StrokeStyle) override
    {
        if (paintingDisabled()) return;
        wkcDrawContextSetStrokeStylePeer(m_data->m_drawcontext, platformStyle(StrokeStyle::SolidStroke));
        for (auto& seg : lineSegments) {
            WKCFloatPoint p = { origin.x() + seg.start(), origin.y() };
            wkcDrawContextDrawLineForTextPeer(m_data->m_drawcontext, &p, seg.end() - seg.start(), isPrinting ? 1 : 0);
        }
    }

    void drawDotsForDocumentMarker(const FloatRect&, DocumentMarkerLineStyle) override { notImplemented(); }

    // --- Image drawing (stubbed) ---

    void drawNativeImage(const NativeImage&, const FloatRect&, const FloatRect&, ImagePaintingOptions = { }) override { notImplemented(); }

    void drawPattern(const NativeImage&, const FloatRect&, const FloatRect&, const AffineTransform&, const FloatPoint&, const FloatSize&, ImagePaintingOptions = { }) override { notImplemented(); }

    // --- Line style ---

    void setLineCap(LineCap cap) override
    {
        int type = WKC_LINECAP_BUTTCAP;
        switch (cap) {
        case LineCap::Butt:   type = WKC_LINECAP_BUTTCAP;   break;
        case LineCap::Round:  type = WKC_LINECAP_ROUNDCAP;  break;
        case LineCap::Square: type = WKC_LINECAP_SQUARECAP; break;
        }
        wkcDrawContextSetLineCapPeer(m_data->m_drawcontext, type);
    }

    void setLineJoin(LineJoin join) override
    {
        int type = WKC_LINEJOIN_MITERJOIN;
        switch (join) {
        case LineJoin::Miter: type = WKC_LINEJOIN_MITERJOIN; break;
        case LineJoin::Round: type = WKC_LINEJOIN_ROUNDJOIN; break;
        case LineJoin::Bevel: type = WKC_LINEJOIN_BEVELJOIN; break;
        }
        wkcDrawContextSetLineJoinPeer(m_data->m_drawcontext, type);
    }

    void setMiterLimit(float lim) override
    {
        wkcDrawContextSetMiterLimitPeer(m_data->m_drawcontext, lim);
    }

    void setLineDash(const DashArray& dashes, float dashOffset) override
    {
        if (dashes.isEmpty()) {
            wkcDrawContextSetLineDashPeer(m_data->m_drawcontext, nullptr, 0, 0);
            return;
        }
        // DashArray = FixedVector<float> — no .data(), iterate manually
        Vector<float> ldash;
        ldash.reserveCapacity(dashes.size());
        for (size_t i = 0; i < dashes.size(); i++)
            ldash.append(dashes[i]);

        bool allzero = true;
        for (float v : ldash) { if (v) { allzero = false; break; } }
        if (allzero) {
            wkcDrawContextSetLineDashPeer(m_data->m_drawcontext, nullptr, 0, 0);
            setStrokeStyle(StrokeStyle::SolidStroke);
            return;
        }
        if (ldash.size() == 1) ldash.append(ldash[0]);
        setStrokeStyle(StrokeStyle::DashedStroke);
        wkcDrawContextSetLineDashPeer(m_data->m_drawcontext, ldash.span().data(), ldash.size(), dashOffset);
    }

    // --- URL ---
    void setURLForRect(const URL&, const FloatRect&) override { notImplemented(); }

private:
    GraphicsContextPlatformPrivate* m_data;
};

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

std::unique_ptr<GraphicsContext> createGraphicsContextWKC(void* drawContext)
{
    return std::make_unique<GraphicsContextWKC>(drawContext);
}

// ---------------------------------------------------------------------------
// Pattern::createPlatformPattern
// ---------------------------------------------------------------------------

PlatformPatternPtr Pattern::createPlatformPattern(const AffineTransform&) const
{
    static WKCPeerPattern s_pattern;
    WKCPeerPattern* pattern = &s_pattern;
    ::memset(pattern, 0, sizeof(WKCPeerPattern));

    // tileImage() returns a SourceImage — get NativeImage from it
    auto ni = tileImage().nativeImage();
    if (!ni) return nullptr;

    auto* img = reinterpret_cast<ImageWKC*>(ni.get());
    if (!img) return nullptr;

    pattern->fType = WKC_PATTERN_IMAGE;
    switch (img->type()) {
    case ImageWKC::EColorARGB8888:
        pattern->u.fImage.fType = WKC_IMAGETYPE_ARGB8888;
        break;
    default:
        return nullptr;
    }
    if (img->hasAlpha())
        pattern->u.fImage.fType |= WKC_IMAGETYPE_FLAG_HASALPHA | WKC_IMAGETYPE_FLAG_HASTRUEALPHA;

    pattern->u.fImage.fBitmap   = img->bitmap();
    pattern->u.fImage.fRowBytes = img->rowbytes();
    WKCFloatRect_SetXYWH(&pattern->u.fImage.fSrcRect, 0, 0, img->size().width(), img->size().height());
    WKCFloatSize_Set(&pattern->u.fImage.fScale, (float)img->scalex(), (float)img->scaley());
    if (!pattern->u.fImage.fScale.fWidth || !pattern->u.fImage.fScale.fHeight)
        return nullptr;
    WKCFloatSize_Set(&pattern->u.fImage.fiScale,
        1.f / pattern->u.fImage.fScale.fWidth,
        1.f / pattern->u.fImage.fScale.fHeight);
    WKCFloatPoint_Set(&pattern->u.fImage.fPhase, 0.f, 0.f);
    WKCFloatSize_Set(&pattern->u.fImage.fiTransform, 1.f, 1.f);

    pattern->u.fImage.fRepeatX = repeatX();
    pattern->u.fImage.fRepeatY = repeatY();
    pattern->u.fImage.fFontId  = const_cast<AffineTransform*>(&patternSpaceTransform());

    return reinterpret_cast<PlatformPatternPtr>(pattern);
}

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
