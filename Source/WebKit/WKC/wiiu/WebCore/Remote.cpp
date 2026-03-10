/*
 * Remote.cpp
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

#include "config.h"
#include "Remote.h"
#include "private/wkcwiiuremoteprivate.h"

namespace WebCore {

Remote::Remote() : m_isEnabled(0)
    , m_isDataValid(0)
    , m_isCursorViewable(0)
    , m_dpdX(0.0)
    , m_dpdY(0.0)
    , m_hold(0)
    , m_dpdRollX(0.0)
    , m_dpdRollY(0.0)
    , m_dpdDistance(0.0)
    , m_dpdValidity(0)
    , m_contentX(0)
    , m_contentY(0)
    , m_accX(0.0)
    , m_accY(0.0)
    , m_accZ(0.0)
    , m_mplsVelX(0.0)
    , m_mplsVelY(0.0)
    , m_mplsVelZ(0.0)
    , m_mplsAngX(0.0)
    , m_mplsAngY(0.0)
    , m_mplsAngZ(0.0)
    , m_mplsDirXx(0.0)
    , m_mplsDirXy(0.0)
    , m_mplsDirXz(0.0)
    , m_mplsDirYx(0.0)
    , m_mplsDirYy(0.0)
    , m_mplsDirYz(0.0)
    , m_mplsDirZx(0.0)
    , m_mplsDirZy(0.0)
    , m_mplsDirZz(0.0)
{
}

Remote*
Remote::update(int id)
{
    WKC::WKCWiiuRemote r = { };

    WKC::WKCWiiuRemoteUpdate( id, &r );
    
    this->m_isEnabled        = r.m_isEnabled;       
    this->m_isDataValid      = r.m_isDataValid;
    this->m_isCursorViewable = r.m_isCursorViewable;
    this->m_dpdX             = r.m_dpdX;
    this->m_dpdY             = r.m_dpdY;
    this->m_hold             = r.m_hold;
    this->m_dpdRollX         = r.m_dpdRollX;
    this->m_dpdRollY         = r.m_dpdRollY;
    this->m_dpdDistance      = r.m_dpdDistance;
    this->m_dpdValidity      = r.m_dpdValidity;
    this->m_contentX         = r.m_contentX;
    this->m_contentY         = r.m_contentY;
    this->m_accX             = r.m_accX;
    this->m_accY             = r.m_accY;
    this->m_accZ             = r.m_accZ;
    this->m_mplsVelX         = r.m_mplsVelX;
    this->m_mplsVelY         = r.m_mplsVelY;
    this->m_mplsVelZ         = r.m_mplsVelZ;
    this->m_mplsAngX         = r.m_mplsAngX;
    this->m_mplsAngY         = r.m_mplsAngY;
    this->m_mplsAngZ         = r.m_mplsAngZ;
    this->m_mplsDirXx        = r.m_mplsDirXx;
    this->m_mplsDirXy        = r.m_mplsDirXy;
    this->m_mplsDirXz        = r.m_mplsDirXz;
    this->m_mplsDirYx        = r.m_mplsDirYx;
    this->m_mplsDirYy        = r.m_mplsDirYy;
    this->m_mplsDirYz        = r.m_mplsDirYz;
    this->m_mplsDirZx        = r.m_mplsDirZx;
    this->m_mplsDirZy        = r.m_mplsDirZy;
    this->m_mplsDirZz        = r.m_mplsDirZz;
    
    return this;
}

void
Remote::setCursorViewable(int id, bool viewable)
{
    WKC::WKCWiiuRemotesetCursorViewable( id, viewable );
}

} //namespace
