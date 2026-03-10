/*
 * WiiuGamePad.h
 *
 * Copyright(c) 2012-2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef WiiuGamePad_h
#define WiiuGamePad_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include "PlatformString.h"

namespace WebCore {

/** 
    js extention GamePad api for wiiu.
    @deprecated this class is former custom api for wiiu. 
    use Gamepad class instead. 
    this class remains only to keep backward compatibility.
*/
class WiiuGamePad : public RefCounted<WiiuGamePad> {
public:
    static PassRefPtr<WiiuGamePad> create() { return adoptRef(new WiiuGamePad()); }

    WiiuGamePad* update();

   // getter functions
    long   isEnabled() { return m_isEnabled; }
    long   isDataValid() { return m_isDataValid; }
    unsigned short tpX() { return m_tpX; }
    unsigned short tpY() { return m_tpY; }
    unsigned short tpValidity() { return m_tpValidity; }
    unsigned short tpTouch() { return m_tpTouch; }
    long   contentX() { return m_contentX; }
    long   contentY() { return m_contentY; }
    long   hold() { return m_hold; }
    double lStickX() { return m_lStickX; }
    double lStickY() { return m_lStickY; }
    double rStickX() { return m_rStickX; }
    double rStickY() { return m_rStickY; }
    double accX() { return m_accX; }
    double accY() { return m_accY; }
    double accZ() { return m_accZ; }
    double gyroX() { return m_gyroX; }
    double gyroY() { return m_gyroY; }
    double gyroZ() { return m_gyroZ; }
    double angleX() { return m_angleX; }
    double angleY() { return m_angleY; }
    double angleZ() { return m_angleZ; }
    double dirXx() { return m_dirXx; }
    double dirXy() { return m_dirXy; }
    double dirXz() { return m_dirXz; }
    double dirYx() { return m_dirYx; }
    double dirYy() { return m_dirYy; }
    double dirYz() { return m_dirYz; }
    double dirZx() { return m_dirZx; }
    double dirZy() { return m_dirZy; }
    double dirZz() { return m_dirZz; }

private:
    WiiuGamePad();

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
};

} // namespace WebCore

#endif // WiiuGamePad_h
