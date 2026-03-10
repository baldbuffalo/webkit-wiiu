/*
 *  wkcwiiuremoteproc.h
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

#ifndef _WKC_WIIU_REMOTE_PROC_H_
#define _WKC_WIIU_REMOTE_PROC_H_

#include <wkc/wkcbase.h>

namespace WKC {

typedef struct WKCWiiuRemote_{
    long   m_isEnabled;
    long   m_isDataValid;
    long   m_isCursorViewable;
    double m_dpdX;
    double m_dpdY;
    long   m_hold;
    double m_dpdRollX;
    double m_dpdRollY;
    double m_dpdDistance;
    long   m_dpdValidity;
    long   m_contentX;
    long   m_contentY;
    double m_accX;
    double m_accY;
    double m_accZ;
    double m_mplsVelX;
    double m_mplsVelY;
    double m_mplsVelZ;
    double m_mplsAngX;
    double m_mplsAngY;
    double m_mplsAngZ;
    double m_mplsDirXx;
    double m_mplsDirXy;
    double m_mplsDirXz;
    double m_mplsDirYx;
    double m_mplsDirYy;
    double m_mplsDirYz;
    double m_mplsDirZx;
    double m_mplsDirZy;
    double m_mplsDirZz;
} WKCWiiuRemote;

typedef void (*wkcWiiuRemoteUpdateProc) ( int in_id, WKCWiiuRemote* out_data );
typedef void (*wkcWiiuRemoteSetCursorViewableProc) ( int in_id, bool in_viewable );

typedef struct WKCWiiuRemoteCallbacks_{
    wkcWiiuRemoteUpdateProc fUpdate;
    wkcWiiuRemoteSetCursorViewableProc fSetCursorViewable;
} WKCWiiuRemoteCallbacks;

WKC_API void WKCWiiuRemoteSetCallbacks(WKCWiiuRemoteCallbacks* in_callbacks);

} // namespace

#endif // _WKC_WIIU_REMOTE_PROC_H_
