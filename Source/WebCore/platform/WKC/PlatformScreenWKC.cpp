/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "PlatformScreen.h"

#include "FloatRect.h"
#include "HostWindow.h"
#include "ScrollView.h"
#include "Widget.h"

namespace WebCore {

static int gScreenHorizontalDPI    = 96;
static int gScreenVerticalDPI      = 96;
static int gScreenWidth            = 0;
static int gScreenHeight           = 0;
static int gAvailableScreenWidth   = 0;
static int gAvailableScreenHeight  = 0;
static int gScreenDepth            = 24;
static int gScreenDepthPerComponent = 8;
static bool gIsMonochrome          = false;

int screenHorizontalDPI(Widget* widget)
{
    return widget ? gScreenHorizontalDPI : 0;
}

int screenVerticalDPI(Widget* widget)
{
    return widget ? gScreenVerticalDPI : 0;
}

int screenDepth(Widget* widget)
{
    return widget ? gScreenDepth : 0;
}

int screenDepthPerComponent(Widget* widget)
{
    return widget ? gScreenDepthPerComponent : 0;
}

bool screenIsMonochrome(Widget* widget)
{
    return widget ? gIsMonochrome : false;
}

bool screenHasInvertedColors()
{
    return false;
}

FloatRect screenRect(Widget* widget)
{
    if (!widget)
        return FloatRect();
    return FloatRect(0, 0, gScreenWidth, gScreenHeight);
}

FloatRect screenAvailableRect(Widget* widget)
{
    if (!widget)
        return FloatRect();
    return FloatRect(0, 0, gAvailableScreenWidth, gAvailableScreenHeight);
}

// WKC platform setters — called by the embedder to configure screen params
void setScreenDPI(int horizontaldpi, int verticaldpi)
{
    gScreenHorizontalDPI = horizontaldpi;
    gScreenVerticalDPI   = verticaldpi;
}

void setScreenSizeWKC(const IntSize& size)
{
    gScreenWidth  = size.width();
    gScreenHeight = size.height();
}

void setAvailableScreenSize(const IntSize& size)
{
    gAvailableScreenWidth  = size.width();
    gAvailableScreenHeight = size.height();
}

void setScreenDepth(int depth, int depthPerComponent)
{
    gScreenDepth            = depth;
    gScreenDepthPerComponent = depthPerComponent;
}

void setIsMonochrome(bool monochrome)
{
    gIsMonochrome = monochrome;
}

} // namespace WebCore
