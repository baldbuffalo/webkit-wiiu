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
    : m_type(Type::Custom)
    , m_image(image)
    , m_hotSpot(determineHotSpot(image, hotSpot))
{
}

Cursor::Cursor(Type type)
    : m_type(type)
{
}

void Cursor::ensurePlatformCursor() const
{
    if (m_platformCursor)
        return;

    if (m_type == Type::Custom) {
        WKC::WKCPlatformCursor* c = new WKC::WKCPlatformCursor(WKC::ECursorTypeCustom);
        if (m_image) {
            auto nativeImage = m_image->nativeImage();
            ImageWKC* wi = nativeImage ? reinterpret_cast<ImageWKC*>(nativeImage.get()) : nullptr;
            if (wi) {
                wi->ref();
                c->fBitmap = wi->bitmap();
                c->fRowBytes = wi->rowbytes();
                c->fBPP = wi->bpp();
                c->fSize.fWidth = m_image->size().width();
                c->fSize.fHeight = m_image->size().height();
                c->fHotSpot.fX = m_hotSpot.x();
                c->fHotSpot.fY = m_hotSpot.y();
                c->fData = reinterpret_cast<void*>(wi);
            }
        }
        m_platformCursor = reinterpret_cast<PlatformCursor>(c);
        return;
    }

    static const WKC::CursorType typeMap[] = {
        WKC::ECursorTypePointer,              // Invalid — fallback
        WKC::ECursorTypePointer,              // Pointer
        WKC::ECursorTypeCross,                // Cross
        WKC::ECursorTypeHand,                 // Hand
        WKC::ECursorTypeIBeam,                // IBeam
        WKC::ECursorTypeWait,                 // Wait
        WKC::ECursorTypeHelp,                 // Help
        WKC::ECursorTypeEastResize,           // EastResize
        WKC::ECursorTypeNorthResize,          // NorthResize
        WKC::ECursorTypeNorthEastResize,      // NorthEastResize
        WKC::ECursorTypeNorthWestResize,      // NorthWestResize
        WKC::ECursorTypeSouthResize,          // SouthResize
        WKC::ECursorTypeSouthEastResize,      // SouthEastResize
        WKC::ECursorTypeSouthWestResize,      // SouthWestResize
        WKC::ECursorTypeWestResize,           // WestResize
        WKC::ECursorTypeNorthSouthResize,     // NorthSouthResize
        WKC::ECursorTypeEastWestResize,       // EastWestResize
        WKC::ECursorTypeNorthEastSouthWestResize, // NorthEastSouthWestResize
        WKC::ECursorTypeNorthWestSouthEastResize, // NorthWestSouthEastResize
        WKC::ECursorTypeColumnResize,         // ColumnResize
        WKC::ECursorTypeRowResize,            // RowResize
        WKC::ECursorTypeMiddlePanning,        // MiddlePanning
        WKC::ECursorTypeEastPanning,          // EastPanning
        WKC::ECursorTypeNorthPanning,         // NorthPanning
        WKC::ECursorTypeNorthEastPanning,     // NorthEastPanning
        WKC::ECursorTypeNorthWestPanning,     // NorthWestPanning
        WKC::ECursorTypeSouthPanning,         // SouthPanning
        WKC::ECursorTypeSouthEastPanning,     // SouthEastPanning
        WKC::ECursorTypeSouthWestPanning,     // SouthWestPanning
        WKC::ECursorTypeWestPanning,          // WestPanning
        WKC::ECursorTypeMove,                 // Move
        WKC::ECursorTypeVerticalText,         // VerticalText
        WKC::ECursorTypeCell,                 // Cell
        WKC::ECursorTypeContextMenu,          // ContextMenu
        WKC::ECursorTypeAlias,                // Alias
        WKC::ECursorTypeProgress,             // Progress
        WKC::ECursorTypeNoDrop,               // NoDrop
        WKC::ECursorTypeCopy,                 // Copy
        WKC::ECursorTypeNone,                 // None
        WKC::ECursorTypeNotAllowed,           // NotAllowed
        WKC::ECursorTypeZoomIn,               // ZoomIn
        WKC::ECursorTypeZoomOut,              // ZoomOut
        WKC::ECursorTypeGrab,                 // Grab
        WKC::ECursorTypeGrabbing,             // Grabbing
        WKC::ECursorTypeCustom,               // Custom
    };

    auto index = static_cast<size_t>(m_type);
    WKC::CursorType wkcType = (index < std::size(typeMap)) ? typeMap[index] : WKC::ECursorTypePointer;
    m_platformCursor = reinterpret_cast<PlatformCursor>(new WKC::WKCPlatformCursor(wkcType));
}

PlatformCursor Cursor::platformCursor() const
{
    ensurePlatformCursor();
    return m_platformCursor;
}

void Cursor::setAsPlatformCursor() const
{
    notImplemented();
}

const Cursor& Cursor::fromType(Cursor::Type type)
{
    static NeverDestroyed<Cursor> cursors[static_cast<size_t>(Type::Custom) + 1];
    auto index = static_cast<size_t>(type);
    ASSERT(index <= static_cast<size_t>(Type::Custom));
    if (cursors[index].get().m_type == Type::Invalid)
        cursors[index].get() = Cursor(type);
    return cursors[index];
}

#define WKC_CURSOR(name, type) \
    const Cursor& name##Cursor() { \
        static NeverDestroyed<Cursor> cursor(Cursor::Type::type); \
        return cursor; \
    }

WKC_CURSOR(pointer,                  Pointer)
WKC_CURSOR(cross,                    Cross)
WKC_CURSOR(hand,                     Hand)
WKC_CURSOR(move,                     Move)
WKC_CURSOR(iBeam,                    IBeam)
WKC_CURSOR(wait,                     Wait)
WKC_CURSOR(help,                     Help)
WKC_CURSOR(eastResize,               EastResize)
WKC_CURSOR(northResize,              NorthResize)
WKC_CURSOR(northEastResize,          NorthEastResize)
WKC_CURSOR(northWestResize,          NorthWestResize)
WKC_CURSOR(southResize,              SouthResize)
WKC_CURSOR(southEastResize,          SouthEastResize)
WKC_CURSOR(southWestResize,          SouthWestResize)
WKC_CURSOR(westResize,               WestResize)
WKC_CURSOR(northSouthResize,         NorthSouthResize)
WKC_CURSOR(eastWestResize,           EastWestResize)
WKC_CURSOR(northEastSouthWestResize, NorthEastSouthWestResize)
WKC_CURSOR(northWestSouthEastResize, NorthWestSouthEastResize)
WKC_CURSOR(columnResize,             ColumnResize)
WKC_CURSOR(rowResize,                RowResize)
WKC_CURSOR(middlePanning,            MiddlePanning)
WKC_CURSOR(eastPanning,              EastPanning)
WKC_CURSOR(northPanning,             NorthPanning)
WKC_CURSOR(northEastPanning,         NorthEastPanning)
WKC_CURSOR(northWestPanning,         NorthWestPanning)
WKC_CURSOR(southPanning,             SouthPanning)
WKC_CURSOR(southEastPanning,         SouthEastPanning)
WKC_CURSOR(southWestPanning,         SouthWestPanning)
WKC_CURSOR(westPanning,              WestPanning)
WKC_CURSOR(verticalText,             VerticalText)
WKC_CURSOR(cell,                     Cell)
WKC_CURSOR(contextMenu,              ContextMenu)
WKC_CURSOR(noDrop,                   NoDrop)
WKC_CURSOR(notAllowed,               NotAllowed)
WKC_CURSOR(progress,                 Progress)
WKC_CURSOR(alias,                    Alias)
WKC_CURSOR(zoomIn,                   ZoomIn)
WKC_CURSOR(zoomOut,                  ZoomOut)
WKC_CURSOR(copy,                     Copy)
WKC_CURSOR(none,                     None)
WKC_CURSOR(grab,                     Grab)
WKC_CURSOR(grabbing,                 Grabbing)

#undef WKC_CURSOR

} // namespace WebCore
