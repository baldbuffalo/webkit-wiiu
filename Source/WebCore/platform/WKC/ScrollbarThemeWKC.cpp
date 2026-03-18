/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "GraphicsContext.h"
#include "IntRect.h"
#include "Scrollbar.h"
#include "ScrollbarTheme.h"
#include "ScrollableArea.h"

#include <wkc/wkcgpeer.h>
#include <wkc/wkcpeer.h>

namespace WebCore {

class ScrollbarThemeWKC final : public ScrollbarTheme {
public:
    ~ScrollbarThemeWKC() override = default;

    int scrollbarThickness(ScrollbarWidth = ScrollbarWidth::Auto, ScrollbarExpansionState = ScrollbarExpansionState::Expanded) override;

    IntRect backButtonRect(Scrollbar&, ScrollbarPart, bool = false) override;
    IntRect forwardButtonRect(Scrollbar&, ScrollbarPart, bool = false) override;
    IntRect trackRect(Scrollbar&, bool = false) override;

    void paintScrollCorner(ScrollableArea&, GraphicsContext&, const IntRect&) override;

protected:
    bool hasButtons(Scrollbar&) override;
    bool hasThumb(Scrollbar&) override;

    int minimumThumbLength(Scrollbar&) override;

    void paintScrollbarBackground(GraphicsContext&, Scrollbar&) override { }
    void paintTrackBackground(GraphicsContext&, Scrollbar&, const IntRect&) override;
    void paintTrackPiece(GraphicsContext&, Scrollbar&, const IntRect&, ScrollbarPart) override { }
    void paintButton(GraphicsContext&, Scrollbar&, const IntRect&, ScrollbarPart) override;
    void paintThumb(GraphicsContext&, Scrollbar&, const IntRect&) override;
    void paintTickmarks(GraphicsContext&, Scrollbar&, const IntRect&) override { }
};

ScrollbarTheme& ScrollbarTheme::nativeTheme()
{
    static ScrollbarThemeWKC* gTheme = new ScrollbarThemeWKC;
    return *gTheme;
}

// ─── Helper to do a 9-slice bitmap blit via WKC peer ───────────────────────

static void _bitblt(void* ctx, int type, void* bitmap, int rowbytes,
                    void* mask, int maskrowbytes,
                    const WKCFloatRect* src, const WKCFloatRect* dest, int op)
{
    WKCPeerImage img;
    memset(&img, 0, sizeof(img));
    img.fType       = type;
    img.fBitmap     = bitmap;
    img.fRowBytes   = rowbytes;
    img.fMask       = mask;
    img.fMaskRowBytes = maskrowbytes;
    img.fSrcRect    = *src;
    img.fScale.fWidth  = 1.f;
    img.fScale.fHeight = 1.f;
    img.fiScale.fWidth = 1.f;
    img.fiScale.fHeight = 1.f;
    img.fPhase.fX   = 0.f;
    img.fPhase.fY   = 0.f;
    img.fiTransform.fWidth  = 1.f;
    img.fiTransform.fHeight = 1.f;
    wkcDrawContextBitBltPeer(ctx, &img, dest, op);
}

static void drawScalingBitmapPeer(void* ctx, void* bitmap, int rowbytes,
                                   WKCSize* sz, const WKCPoint* pts,
                                   const WKCRect* dst, int op)
{
    const int T = WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN;

    auto blit = [&](float sx, float sy, float sw, float sh,
                    float dx, float dy, float dw, float dh) {
        if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
        WKCFloatRect src  = { sx, sy, sw, sh };
        WKCFloatRect dest = { dx, dy, dw, dh };
        _bitblt(ctx, T, bitmap, rowbytes, nullptr, 0, &src, &dest, op);
    };

    float sw = (float)sz->fWidth;
    float sh = (float)sz->fHeight;
    float dx = (float)dst->fX;
    float dy = (float)dst->fY;
    float dw = (float)dst->fWidth;
    float dh = (float)dst->fHeight;

    float p0x = (float)pts[0].fX, p0y = (float)pts[0].fY;
    float p1x = (float)pts[1].fX, p1y = (float)pts[1].fY;
    float p2x = (float)pts[2].fX, p2y = (float)pts[2].fY;
    float p3x = (float)pts[3].fX, p3y = (float)pts[3].fY;

    // corners
    blit(0,   0,   p0x,      p0y,      dx,               dy,               p0x,               p0y);
    blit(p1x, 0,   sw-p1x,   p0y,      dx+dw-(sw-p1x),   dy,               sw-p1x,            p0y);
    blit(0,   p2y, p2x,      sh-p2y,   dx,               dy+dh-(sh-p2y),   p2x,               sh-p2y);
    blit(p3x, p3y, sw-p3x,   sh-p3y,   dx+dw-(sw-p3x),   dy+dh-(sh-p3y),   sw-p3x,            sh-p3y);
    // edges
    blit(p0x, 0,   p1x-p0x,  p0y,      dx+p0x,           dy,               dw-p0x-(sw-p1x),   p0y);
    blit(0,   p0y, p0x,      p2y-p0y,  dx,               dy+p0y,           p0x,               dh-p0y-(sh-p2y));
    blit(p1x, p1y, sw-p1x,   p3y-p1y,  dx+dw-(sw-p1x),   dy+p1y,           sw-p1x,            dh-p1y-(sh-p3y));
    blit(p2x, p2y, p3x-p2x,  sh-p2y,   dx+p2x,           dy+dh-(sh-p2y),   dw-p2x-(sw-p3x),   sh-p2y);
    // center
    blit(p0x, p0y, p3x-p0x,  p3y-p0y,  dx+p0x,           dy+p0y,           dw-p0x-(sw-p3x),   dh-p0y-(sh-p3y));
}

// ─── ScrollbarThemeWKC implementation ──────────────────────────────────────

int ScrollbarThemeWKC::scrollbarThickness(ScrollbarWidth controlSize, ScrollbarExpansionState)
{
    if (controlSize == ScrollbarWidth::None)
        return 0;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w, &h);
    return (int)h;
}

bool ScrollbarThemeWKC::hasButtons(Scrollbar& scrollbar)
{
    unsigned int dummy = 0, ss = 0;
    int ds;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &ss, &dummy);
        ds = scrollbar.width();
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &dummy, &ss);
        ds = scrollbar.height();
    }
    return ds >= 2 * (int)ss;
}

bool ScrollbarThemeWKC::hasThumb(Scrollbar& scrollbar)
{
    if (!scrollbar.enabled()) return false;
    unsigned int dummy = 0, ss = 0, ts = 0;
    int ds;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &ss, &dummy);
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_THUMB, &ts, &dummy);
        ds = scrollbar.width();
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &dummy, &ss);
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_THUMB, &dummy, &ts);
        ds = scrollbar.height();
    }
    return hasButtons(scrollbar) ? ds >= 2*(int)ss+(int)ts : ds >= (int)ts;
}

IntRect ScrollbarThemeWKC::backButtonRect(Scrollbar& scrollbar, ScrollbarPart part, bool)
{
    if (part == BackButtonEndPart)
        return IntRect();
    unsigned int w = 0, h = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal)
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w, &h);
    else
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &w, &h);
    return IntRect(scrollbar.x(), scrollbar.y(), (int)w, (int)h);
}

IntRect ScrollbarThemeWKC::forwardButtonRect(Scrollbar& scrollbar, ScrollbarPart part, bool)
{
    if (part == ForwardButtonStartPart)
        return IntRect();
    unsigned int w = 0, h = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w, &h);
        return IntRect(scrollbar.x() + scrollbar.width() - (int)w, scrollbar.y(), (int)w, (int)h);
    }
    wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w, &h);
    return IntRect(scrollbar.x(), scrollbar.y() + scrollbar.height() - (int)h, (int)w, (int)h);
}

IntRect ScrollbarThemeWKC::trackRect(Scrollbar& scrollbar, bool)
{
    if (!hasButtons(scrollbar))
        return scrollbar.frameRect();
    unsigned int w0 = 0, h0 = 0, w1 = 0, h1 = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT,  &w0, &h0);
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w1, &h1);
        return IntRect(scrollbar.x()+(int)w0, scrollbar.y(), scrollbar.width()-(int)w0-(int)w1, (int)h0);
    }
    wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP,   &w0, &h0);
    wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w1, &h1);
    return IntRect(scrollbar.x(), scrollbar.y()+(int)h0, (int)w0, scrollbar.height()-(int)h0-(int)h1);
}

int ScrollbarThemeWKC::minimumThumbLength(Scrollbar& scrollbar)
{
    unsigned int ss = 0, dummy = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal)
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_THUMB, &ss, &dummy);
    else
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_THUMB, &dummy, &ss);
    return (int)ss;
}

void ScrollbarThemeWKC::paintTrackBackground(GraphicsContext& context, Scrollbar& scrollbar, const IntRect& r)
{
    void* ctx = context.platformContext();
    if (!ctx) return;

    int index = scrollbar.orientation() == ScrollbarOrientation::Horizontal
        ? (scrollbar.enabled() ? WKC_IMAGE_H_SCROLLBAR_BACKGROUND : WKC_IMAGE_H_SCROLLBAR_BACKGROUND_DISABLED)
        : (scrollbar.enabled() ? WKC_IMAGE_V_SCROLLBAR_BACKGROUND : WKC_IMAGE_V_SCROLLBAR_BACKGROUND_DISABLED);

    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return;

    WKCSize sz  = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, (int)(w*4), &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintButton(GraphicsContext& context, Scrollbar& scrollbar,
                                     const IntRect& r, ScrollbarPart part)
{
    void* ctx = context.platformContext();
    if (!ctx || !hasButtons(scrollbar)) return;

    bool horizontal = scrollbar.orientation() == ScrollbarOrientation::Horizontal;
    bool hovered = (part == scrollbar.pressedPart() || part == scrollbar.hoveredPart());
    int index = 0;

    if (scrollbar.enabled()) {
        if (part == BackButtonStartPart)
            index = horizontal ? (hovered ? WKC_IMAGE_H_SCROLLBAR_LEFT_HOVERED  : WKC_IMAGE_H_SCROLLBAR_LEFT)
                               : (hovered ? WKC_IMAGE_V_SCROLLBAR_UP_HOVERED    : WKC_IMAGE_V_SCROLLBAR_UP);
        else if (part == ForwardButtonEndPart)
            index = horizontal ? (hovered ? WKC_IMAGE_H_SCROLLBAR_RIGHT_HOVERED : WKC_IMAGE_H_SCROLLBAR_RIGHT)
                               : (hovered ? WKC_IMAGE_V_SCROLLBAR_DOWN_HOVERED  : WKC_IMAGE_V_SCROLLBAR_DOWN);
    } else {
        if (part == BackButtonStartPart)
            index = horizontal ? WKC_IMAGE_H_SCROLLBAR_LEFT_DISABLED  : WKC_IMAGE_V_SCROLLBAR_UP_DISABLED;
        else if (part == ForwardButtonEndPart)
            index = horizontal ? WKC_IMAGE_H_SCROLLBAR_RIGHT_DISABLED : WKC_IMAGE_V_SCROLLBAR_DOWN_DISABLED;
    }

    if (!index) return;
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return;

    WKCFloatRect src  = { 0, 0, (float)w, (float)h };
    WKCFloatRect dest = { (float)r.x(), (float)r.y(), (float)r.width(), (float)r.height() };
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, (int)(w*4), nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintThumb(GraphicsContext& context, Scrollbar& scrollbar, const IntRect& r)
{
    void* ctx = context.platformContext();
    if (!ctx || !scrollbar.enabled() || !hasThumb(scrollbar)) return;

    bool hovered = (ThumbPart == scrollbar.pressedPart() || ThumbPart == scrollbar.hoveredPart());
    bool horizontal = scrollbar.orientation() == ScrollbarOrientation::Horizontal;
    int index = horizontal ? (hovered ? WKC_IMAGE_H_SCROLLBAR_THUMB_HOVERED : WKC_IMAGE_H_SCROLLBAR_THUMB)
                           : (hovered ? WKC_IMAGE_V_SCROLLBAR_THUMB_HOVERED : WKC_IMAGE_V_SCROLLBAR_THUMB);

    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return;

    WKCSize sz   = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, (int)(w*4), &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintScrollCorner(ScrollableArea&, GraphicsContext& context, const IntRect& r)
{
    void* ctx = context.platformContext();
    if (!ctx) return;

    const int index = WKC_IMAGE_SCROLLBAR_CROSS_CORNER;
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return;

    WKCFloatRect src  = { 0, 0, (float)w, (float)h };
    WKCFloatRect dest = { (float)r.x(), (float)r.y(), (float)r.width(), (float)r.height() };
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, (int)(w*4), nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

} // namespace WebCore
