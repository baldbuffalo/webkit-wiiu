/*
 * Remote.h
 *
 * Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef Remote_h
#define Remote_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include "PlatformString.h"

namespace WebCore {

class Remote : public RefCounted<Remote> {
public:
    static PassRefPtr<Remote> create() { return adoptRef(new Remote()); }

    Remote* update(int id);
    void setCursorViewable(int id, bool viewable);

    // getter functions
    long   isEnabled() { return m_isEnabled; }
    long   isDataValid() { return m_isDataValid; }
    long   isCursorViewable() { return m_isCursorViewable; }
    double dpdX() { return m_dpdX; }
    double dpdY() { return m_dpdY; }
    long   hold() { return m_hold; }
    double dpdRollX() { return m_dpdRollX; }
    double dpdRollY() { return m_dpdRollY; }
    double dpdDistance() { return m_dpdDistance; }
    long   dpdValidity() { return m_dpdValidity; }
    long   contentX() { return m_contentX; }
    long   contentY() { return m_contentY; }
    double accX() { return m_accX; }
    double accY() { return m_accY; }
    double accZ() { return m_accZ; }
    double mplsVelX() { return m_mplsVelX; }
    double mplsVelY() { return m_mplsVelY; }
    double mplsVelZ() { return m_mplsVelZ; }
    double mplsAngX() { return m_mplsAngX; }
    double mplsAngY() { return m_mplsAngY; }
    double mplsAngZ() { return m_mplsAngZ; }
    double mplsDirXx() { return m_mplsDirXx; }
    double mplsDirXy() { return m_mplsDirXy; }
    double mplsDirXz() { return m_mplsDirXz; }
    double mplsDirYx() { return m_mplsDirYx; }
    double mplsDirYy() { return m_mplsDirYy; }
    double mplsDirYz() { return m_mplsDirYz; }
    double mplsDirZx() { return m_mplsDirZx; }
    double mplsDirZy() { return m_mplsDirZy; }
    double mplsDirZz() { return m_mplsDirZz; }
   
private:
    Remote();
    
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
};

} // namespace WebCore

#endif // Remote_h
