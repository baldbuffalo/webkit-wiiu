/*
 *  wkcwiiugamepadproc.h
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

#ifndef _WKC_WIIU_GAMEPAD_PROC_H_
#define _WKC_WIIU_GAMEPAD_PROC_H_

#include <wkc/wkcbase.h>

namespace WKC {

typedef struct WKCWiiuGamePad_ {
    long   m_isEnabled;
    long   m_isDataValid;
    unsigned short m_tpX;
    unsigned short m_tpY;
    unsigned short m_tpValidity;
    unsigned short m_tpTouch;
    long   m_contentX;
    long   m_contentY;
    long   m_hold;
    double m_lStickX;
    double m_lStickY;
    double m_rStickX;
    double m_rStickY;
    double m_accX;
    double m_accY;
    double m_accZ;
    double m_gyroX;
    double m_gyroY;
    double m_gyroZ;
    double m_angleX;
    double m_angleY;
    double m_angleZ;
    double m_dirXx;
    double m_dirXy;
    double m_dirXz;
    double m_dirYx;
    double m_dirYy;
    double m_dirYz;
    double m_dirZx;
    double m_dirZy;
    double m_dirZz;
} WKCWiiuGamePad;

typedef void (*wkcWiiuGamePadUpdateProc) (WKCWiiuGamePad* out_data);

typedef struct WKCWiiuGamePadCallbacks_ {
    wkcWiiuGamePadUpdateProc fUpdate;
} WKCWiiuGamePadCallbacks;

WKC_API void WKCWiiuGamePadSetCallbacks(WKCWiiuGamePadCallbacks* in_callbacks);

} // namespace

#endif // _WKC_WIIU_GAMEPAD_PROC_H_
