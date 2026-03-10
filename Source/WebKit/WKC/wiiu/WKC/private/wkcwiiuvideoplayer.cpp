/*
 *  wkcwiiuvideoplayer.cpp
 *
 *  Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
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

#include <wkc/wkcbase.h>
#include "wkcwiiuvideoplayerproc.h"
#include "wkcwiiuvideoplayerprivate.h"

namespace WKC {

static WKCWiiuVideoPlayerCallbacks g_callbacks;

WKC_API void
WKCWiiuVideoPlayerSetCallbacks(WKCWiiuVideoPlayerCallbacks* in_callbacks)
{
    g_callbacks.fEnd = in_callbacks->fEnd;
    g_callbacks.fViewMode = in_callbacks->fViewMode;
    g_callbacks.fSetViewMode = in_callbacks->fSetViewMode;

    return;
}

bool
WKCWiiuEndVideoPlayer()
{
    if (g_callbacks.fEnd) {
        return (*g_callbacks.fEnd)();
    }

    return false;
}

long
WKCWiiuVideoPlayerViewMode()
{
    if(!g_callbacks.fViewMode)
        return 0;

    return g_callbacks.fViewMode() ? 1 : 0;
}

void
WKCWiiuVideoPlayerSetViewMode(long in_mode)
{
    if(!g_callbacks.fSetViewMode)
        return;

    g_callbacks.fSetViewMode(in_mode);
}

} // namespace
