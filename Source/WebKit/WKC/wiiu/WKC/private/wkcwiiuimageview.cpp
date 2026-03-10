/*
 *  wkcwiiuimageview.cpp
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

#include <cstring>
#include <wkc/wkcbase.h>
#include "wkcwiiuimageviewproc.h"
#include "wkcwiiuimageviewprivate.h"

namespace WKC {

static WKCWiiuImageViewProcs g_procs = {0};
static void* g_arg = 0;

WKC_API bool
WKCWiiuImageViewProcInitialize(void* in_arg, WKCWiiuImageViewProcs* in_procs)
{
    g_arg = in_arg;
    memcpy(&g_procs, in_procs, sizeof(WKCWiiuImageViewProcs));

    return true;
}

WKC_API void
WKCWiiuImageViewProcFinalize()
{
    memset(&g_procs, 0, sizeof(WKCWiiuImageViewProcs));
    g_arg = 0;
}

bool
WKCWiiuEndImageView()
{
    if (g_procs.fEndProc) {
        return (*g_procs.fEndProc)(g_arg);
    }

    return false;
}

bool
WKCWiiuImageViewChangeViewMode(long in_mode)
{
    if (g_procs.fChangeViewModeProc) {
        return (*g_procs.fChangeViewModeProc)(g_arg, in_mode);
    }

    return false;
}

long
WKCWiiuImageViewViewMode()
{
    if (g_procs.fViewModeProc) {
        return (*g_procs.fViewModeProc)(g_arg) ? 1 : 0;
    }

    return 0;
}

long
WKCWiiuImageViewGetLastError()
{
    if (g_procs.fGetLastErrorProc) {
        return (*g_procs.fGetLastErrorProc)(g_arg);
    }

    return 0;
}

} // namespace
