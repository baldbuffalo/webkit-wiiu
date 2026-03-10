/*
 *  wkcwiiuremote.cpp
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
#include "wkcwiiuremoteproc.h"
#include "wkcwiiuremoteprivate.h"

namespace WKC {

static WKCWiiuRemoteCallbacks g_callbacks;

WKC_API void
WKCWiiuRemoteSetCallbacks(WKCWiiuRemoteCallbacks* in_callbacks)
{
    g_callbacks.fUpdate = in_callbacks->fUpdate;
    g_callbacks.fSetCursorViewable = in_callbacks->fSetCursorViewable;

    return;
}

void
WKCWiiuRemoteUpdate( int in_id, WKCWiiuRemote* out_data )
{
    if( !g_callbacks.fUpdate || !out_data )
        return;

    g_callbacks.fUpdate( in_id, out_data );
    return;
}

void
WKCWiiuRemotesetCursorViewable( int in_id, bool in_viewable )
{
    if( !g_callbacks.fSetCursorViewable )
        return;

    g_callbacks.fSetCursorViewable( in_id, in_viewable );
    return;
}

} // namespace
