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
#include "Scrollbar.h"
#include "ScrollbarTheme.h"
#include "ScrollView.h"

#include <wkc/wkcgpeer.h>
#include <wkc/wkcpeer.h>

namespace WebCore {

class ScrollbarThemeWKC final : public ScrollbarTheme {
public:
    ~ScrollbarThemeWKC() override = default;

    int scrollbarThickness(ScrollbarWidth, ScrollbarExpansionState) override;

    IntRect backButtonRect(Scrollbar&, ScrollbarPart, bool painting = false) override;
    IntRect forwardButtonRect(Scrollbar&, ScrollbarPart, bool painting = false) override;
    IntRect trackRect(Scrollbar&, bool painting = false) override;

    void paintScrollCorner(ScrollableArea&, GraphicsContext&, const IntRect&) override;

protected:
    bool hasButtons(Scrollbar&) override;
    bool hasThumb(Scrollbar&) override;

    int minimumThumbLength(Scrollbar&) override;

    void invalidatePart(Scrollbar&, ScrollbarPart) override;

    void paintScrollbarBackground(GraphicsContext&, Scrollbar&) override { }
    void paintTrackBackground(GraphicsContext&, Scrollbar&, const IntRect&) override;
    void paintTrackPiece(GraphicsContext&, Scrollbar&, const IntRect&, ScrollbarPart) override { }
    void paintButton(GraphicsContext&, Scrollbar&, const IntRect&, ScrollbarPart) override;
    void paintThumb(GraphicsContext&, Scrollbar&, const IntRect&) override;
    void paintTickmarks(GraphicsContext&, Scrollbar&, const IntRect&) override { }

    bool paint(Scrollbar&, GraphicsContext&, const IntRect& damageRect) override;
};

ScrollbarTheme& ScrollbarTheme::nativeTheme()
{
    static ScrollbarThemeWKC* gTheme = new ScrollbarThemeWKC;
    return *gTheme;
}

// ─── Drawing helpers ────────────────────────────────────────────────────────

static void _bitblt(void* ctx, int type, void* bitmap, int rowbytes, void* mask, int maskrowbytes,
                    const WKCFloatRect* srcrect, const WKCFloatRect* destrect, int op)
{
    WKCPeerImage img = { 0 };
    img.fType = type;
    img.fBitmap = bitmap;
    img.fRowBytes = rowbytes;
    img.fMask = mask;
    img.fMaskRowBytes = maskrowbytes;
    WKCFloatRect_SetRect(&img.fSrcRect, srcrect);
    WKCFloatSize_Set(&img.fScale, 1, 1);
    WKCFloatSize_Set(&img.fiScale, 1, 1);
    WKCFloatPoint_Set(&img.fPhase, 0, 0);
    WKCFloatSize_Set(&img.fiTransform, 1, 1);
    wkcDrawContextBitBltPeer(ctx, &img, destrect, op);
}

static void drawScalingBitmapPeer(void* ctx, void* bitmap, int rowbytes,
                                   WKCSize* sz, const WKCPoint* pts,
                                   const WKCRect* dst, int op)
{
    WKCFloatRect src, dest;
    const int T = WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN;

#define BLIT(sx,sy,sw,sh, dx,dy,dw,dh) \
    do { src.fX=sx; src.fY=sy; src.fWidth=sw; src.fHeight=sh; \
         dest.fX=dx; dest.fY=dy; dest.fWidth=dw; dest.fHeight=dh; \
         if (sw>0&&sh>0&&dw>0&&dh>0) _bitblt(ctx,T,bitmap,rowbytes,nullptr,0,&src,&dest,op); } while(0)

    // corners
    BLIT(0,0, pts[0].fX,pts[0].fY, dst->fX,dst->fY, pts[0].fX,pts[0].fY);
    BLIT(pts[1].fX,0, sz->fWidth-pts[1].fX,pts[0].fY, dst->fX+dst->fWidth-(sz->fWidth-pts[1].fX),dst->fY, sz->fWidth-pts[1].fX,pts[0].fY);
    BLIT(0,pts[2].fY, pts[2].fX,sz->fHeight-pts[2].fY, dst->fX,dst->fY+dst->fHeight-(sz->fHeight-pts[2].fY), pts[2].fX,sz->fHeight-pts[2].fY);
    BLIT(pts[3].fX,pts[3].fY, sz->fWidth-pts[3].fX,sz->fHeight-pts[3].fY, dst->fX+dst->fWidth-(sz->fWidth-pts[3].fX),dst->fY+dst->fHeight-(sz->fHeight-pts[3].fY), sz->fWidth-pts[3].fX,sz->fHeight-pts[3].fY);
    // edges
    BLIT(pts[0].fX,0, pts[1].fX-pts[0].fX,pts[0].fY, dst->fX+pts[0].fX,dst->fY, dst->fWidth-pts[0].fX-(sz->fWidth-pts[1].fX),pts[0].fY);
    BLIT(0,pts[0].fY, pts[0].fX,pts[2].fY-pts[0].fY, dst->fX,dst->fY+pts[0].fY, pts[0].fX,dst->fHeight-pts[0].fY-(sz->fHeight-pts[2].fY));
    BLIT(pts[1].fX,pts[1].fY, sz->fWidth-pts[1].fX,pts[3].fY-pts[1].fY, dst->fX+dst->fWidth-(sz->fWidth-pts[1].fX),dst->fY+pts[1].fY, sz->fWidth-pts[1].fX,dst->fHeight-pts[1].fY-(sz->fHeight-pts[3].fY));
    BLIT(pts[2].fX,pts[2].fY, pts[3].fX-pts[2].fX,sz->fHeight-pts[2].fY, dst->fX+pts[2].fX,dst->fY+dst->fHeight-(sz->fHeight-pts[2].fY), dst->fWidth-pts[2].fX-(sz->fWidth-pts[3].fX),sz->fHeight-pts[2].fY);
    // center
    BLIT(pts[0].fX,pts[0].fY, pts[3].fX-pts[0].fX,pts[3].fY-pts[0].fY, dst->fX+pts[0].fX,dst->fY+pts[0].fY, dst->fWidth-pts[0].fX-(sz->fWidth-pts[3].fX),dst->fHeight-pts[0].fY-(sz->fHeight-pts[3].fY));
#undef BLIT
}

// ─── Theme implementation ───────────────────────────────────────────────────

int ScrollbarThemeWKC::scrollbarThickness(ScrollbarWidth controlSize, ScrollbarExpansionState)
{
    if (controlSize == ScrollbarWidth::Auto) {
        unsigned int w = 0, h = 0;
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w, &h);
        return h;
    }
    return 0;
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
    return hasButtons(scrollbar) ? ds >= 2 * (int)ss + (int)ts : ds >= (int)ts;
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
    return IntRect(scrollbar.x(), scrollbar.y(), w, h);
}

IntRect ScrollbarThemeWKC::forwardButtonRect(Scrollbar& scrollbar, ScrollbarPart part, bool)
{
    if (part == ForwardButtonStartPart)
        return IntRect();
    unsigned int w = 0, h = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w, &h);
        return IntRect(scrollbar.x() + scrollbar.width() - w, scrollbar.y(), w, h);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w, &h);
        return IntRect(scrollbar.x(), scrollbar.y() + scrollbar.height() - h, w, h);
    }
}

IntRect ScrollbarThemeWKC::trackRect(Scrollbar& scrollbar, bool)
{
    if (!hasButtons(scrollbar))
        return scrollbar.frameRect();

    unsigned int w0 = 0, h0 = 0, w1 = 0, h1 = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal) {
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_LEFT, &w0, &h0);
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_RIGHT, &w1, &h1);
        return IntRect(scrollbar.x() + w0, scrollbar.y(), scrollbar.width() - w0 - w1, h0);
    } else {
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_UP, &w0, &h0);
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_DOWN, &w1, &h1);
        return IntRect(scrollbar.x(), scrollbar.y() + h0, w0, scrollbar.height() - h0 - h1);
    }
}

int ScrollbarThemeWKC::minimumThumbLength(Scrollbar& scrollbar)
{
    unsigned int ss = 0, dummy = 0;
    if (scrollbar.orientation() == ScrollbarOrientation::Horizontal)
        wkcStockImageGetSizePeer(WKC_IMAGE_H_SCROLLBAR_THUMB, &ss, &dummy);
    else
        wkcStockImageGetSizePeer(WKC_IMAGE_V_SCROLLBAR_THUMB, &dummy, &ss);
    return ss;
}

void ScrollbarThemeWKC::invalidatePart(Scrollbar& scrollbar, ScrollbarPart part)
{
    ScrollbarTheme::invalidatePart(scrollbar, part);
}

void ScrollbarThemeWKC::paintTrackBackground(GraphicsContext& context, Scrollbar& scrollbar, const IntRect& r)
{
    void* ctx = context.platformContext();
    if (!ctx) return;

    int index;
    if (scrollbar.enabled())
        index = scrollbar.orientation() == ScrollbarOrientation::Horizontal
            ? WKC_IMAGE_H_SCROLLBAR_BACKGROUND : WKC_IMAGE_V_SCROLLBAR_BACKGROUND;
    else
        index = scrollbar.orientation() == ScrollbarOrientation::Horizontal
            ? WKC_IMAGE_H_SCROLLBAR_BACKGROUND_DISABLED : WKC_IMAGE_V_SCROLLBAR_BACKGROUND_DISABLED;

    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return;

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintButton(GraphicsContext& context, Scrollbar& scrollbar, const IntRect& r, ScrollbarPart part)
{
    void* ctx = context.platformContext();
    if (!ctx) return;
    if (!hasButtons(scrollbar)) return;

    int index = 0;
    bool horizontal = scrollbar.orientation() == ScrollbarOrientation::Horizontal;
    bool hovered = (part == scrollbar.pressedPart() || part == scrollbar.hoveredPart());

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
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

void ScrollbarThemeWKC::paintThumb(GraphicsContext& context, Scrollbar& scrollbar, const IntRect& r)
{
    void* ctx = context.platformContext();
    if (!ctx) return;
    if (!scrollbar.enabled() || !hasThumb(scrollbar)) return;

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

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

bool ScrollbarThemeWKC::paint(Scrollbar& scrollbar, GraphicsContext& context, const IntRect& damageRect)
{
    return ScrollbarTheme::paint(scrollbar, context, damageRect);
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
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
}

} // namespace WebCore
