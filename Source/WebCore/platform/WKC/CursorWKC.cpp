/*
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Christian Dywan <christian@twotoasts.de>
 * All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
#include "Cursor.h"

#include "Image.h"
#include "IntPoint.h"
#include "ImageWKC.h"
#include "NotImplemented.h"
#include "WKCEnums.h"
#include "helpers/ChromeClientIf.h"
#include <wtf/Assertions.h>
#include <wtf/NeverDestroyed.h>

namespace WebCore {

Cursor::Cursor(Image* image, const IntPoint& hotSpot)
    : m_platformCursor(nullptr)
{
    WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor(WKC::ECursorTypeCustom);
    auto nativeImage = image->nativeImage();
    ImageWKC* wi = nativeImage ? reinterpret_cast<ImageWKC*>(nativeImage.get()) : nullptr;
    if (!wi) {
        delete c;
        m_platformCursor = reinterpret_cast<PlatformCursor>(new WKC::WKCPlatformCursor(WKC::ECursorTypePointer));
        return;
    }
    wi->ref();
    c->fBitmap = wi->bitmap();
    c->fRowBytes = wi->rowbytes();
    c->fBPP = wi->bpp();
    c->fSize.fWidth = image->size().width();
    c->fSize.fHeight = image->size().height();
    c->fHotSpot.fX = hotSpot.x();
    c->fHotSpot.fY = hotSpot.y();
    c->fData = reinterpret_cast<void*>(wi);
    m_platformCursor = reinterpret_cast<PlatformCursor>(c);
}

#define WKC_CURSOR(name, type) \
    const Cursor& name##Cursor() { \
        static NeverDestroyed<Cursor> cursor(static_cast<Cursor::Type>(type)); \
        return cursor; \
    }

WKC_CURSOR(pointer,                  WKC::ECursorTypePointer)
WKC_CURSOR(cross,                    WKC::ECursorTypeCross)
WKC_CURSOR(hand,                     WKC::ECursorTypeHand)
WKC_CURSOR(move,                     WKC::ECursorTypeMove)
WKC_CURSOR(iBeam,                    WKC::ECursorTypeIBeam)
WKC_CURSOR(wait,                     WKC::ECursorTypeWait)
WKC_CURSOR(help,                     WKC::ECursorTypeHelp)
WKC_CURSOR(eastResize,               WKC::ECursorTypeEastResize)
WKC_CURSOR(northResize,              WKC::ECursorTypeNorthResize)
WKC_CURSOR(northEastResize,          WKC::ECursorTypeNorthEastResize)
WKC_CURSOR(northWestResize,          WKC::ECursorTypeNorthWestResize)
WKC_CURSOR(southResize,              WKC::ECursorTypeSouthResize)
WKC_CURSOR(southEastResize,          WKC::ECursorTypeSouthEastResize)
WKC_CURSOR(southWestResize,          WKC::ECursorTypeSouthWestResize)
WKC_CURSOR(westResize,               WKC::ECursorTypeWestResize)
WKC_CURSOR(northSouthResize,         WKC::ECursorTypeNorthSouthResize)
WKC_CURSOR(eastWestResize,           WKC::ECursorTypeEastWestResize)
WKC_CURSOR(northEastSouthWestResize, WKC::ECursorTypeNorthEastSouthWestResize)
WKC_CURSOR(northWestSouthEastResize, WKC::ECursorTypeNorthWestSouthEastResize)
WKC_CURSOR(columnResize,             WKC::ECursorTypeColumnResize)
WKC_CURSOR(rowResize,                WKC::ECursorTypeRowResize)
WKC_CURSOR(middlePanning,            WKC::ECursorTypeMiddlePanning)
WKC_CURSOR(eastPanning,              WKC::ECursorTypeEastPanning)
WKC_CURSOR(northPanning,             WKC::ECursorTypeNorthPanning)
WKC_CURSOR(northEastPanning,         WKC::ECursorTypeNorthEastPanning)
WKC_CURSOR(northWestPanning,         WKC::ECursorTypeNorthWestPanning)
WKC_CURSOR(southPanning,             WKC::ECursorTypeSouthPanning)
WKC_CURSOR(southEastPanning,         WKC::ECursorTypeSouthEastPanning)
WKC_CURSOR(southWestPanning,         WKC::ECursorTypeSouthWestPanning)
WKC_CURSOR(westPanning,              WKC::ECursorTypeWestPanning)
WKC_CURSOR(verticalText,             WKC::ECursorTypeVerticalText)
WKC_CURSOR(cell,                     WKC::ECursorTypeCell)
WKC_CURSOR(contextMenu,              WKC::ECursorTypeContextMenu)
WKC_CURSOR(noDrop,                   WKC::ECursorTypeNoDrop)
WKC_CURSOR(copy,                     WKC::ECursorTypeCopy)
WKC_CURSOR(progress,                 WKC::ECursorTypeProgress)
WKC_CURSOR(alias,                    WKC::ECursorTypeAlias)
WKC_CURSOR(none,                     WKC::ECursorTypeNone)
WKC_CURSOR(notAllowed,               WKC::ECursorTypeNotAllowed)
WKC_CURSOR(zoomIn,                   WKC::ECursorTypeZoomIn)
WKC_CURSOR(zoomOut,                  WKC::ECursorTypeZoomOut)
WKC_CURSOR(grab,                     WKC::ECursorTypeGrab)
WKC_CURSOR(grabbing,                 WKC::ECursorTypeGrabbing)

#undef WKC_CURSOR

} // namespace WebCore
