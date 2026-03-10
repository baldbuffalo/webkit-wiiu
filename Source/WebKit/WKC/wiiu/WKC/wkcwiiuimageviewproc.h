/*
 *  wkcwiiuimageviewproc.h
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

#ifndef _WKC_WIIU_IMAGEVIEW_PROC_H_
#define _WKC_WIIU_IMAGEVIEW_PROC_H_

#include <wkc/wkcbase.h>

namespace WKC {

typedef bool (*wkcWiiuEndImageViewProc)(void* in_arg);
typedef bool (*wkcWiiuImageViewChangeViewModeProc)(void* in_arg, bool in_mode);
typedef bool (*wkcWiiuImageViewViewModeProc)(void* in_arg);
typedef long (*wkcWiiuImageViewGetLastErrorProc)(void* in_arg);

struct WKCWiiuImageViewProcs_{
    wkcWiiuEndImageViewProc fEndProc;
    wkcWiiuImageViewChangeViewModeProc fChangeViewModeProc;
    wkcWiiuImageViewViewModeProc fViewModeProc;
    wkcWiiuImageViewGetLastErrorProc fGetLastErrorProc;
};
typedef struct WKCWiiuImageViewProcs_ WKCWiiuImageViewProcs;

enum WKCWiiuImageViewEvent {
    EEventWiiuImageViewStart = 0,
    EEventWiiuImageViewEnd,
    EEventWiiuImageViewChangeViewMode,
    EEventWiiuImageViewChangeContent,
    EEventWiiuImageViewError,
    EEventWiiuImageViews
};

WKC_API bool WKCWiiuImageViewProcInitialize(void* in_arg, WKCWiiuImageViewProcs* in_procs);
WKC_API void WKCWiiuImageViewProcFinalize();

} // namespace

#endif // _WKC_WIIU_IMAGEVIEW_PROC_H_
