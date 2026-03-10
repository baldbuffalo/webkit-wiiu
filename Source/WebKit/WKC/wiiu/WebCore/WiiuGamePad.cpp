/*
 * WiiuGamePad.cpp
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

#include "config.h"
#include "WiiuGamePad.h"
#include "private/wkcwiiugamepadprivate.h"

namespace WebCore {

WiiuGamePad::WiiuGamePad()
    : m_isEnabled(0)
    , m_isDataValid(0)
    , m_tpX(0)
    , m_tpY(0)
    , m_tpValidity(0)
    , m_tpTouch(0)
    , m_contentX(0)
    , m_contentY(0)
    , m_hold(0)
    , m_lStickX(0.0)
    , m_lStickY(0.0)
    , m_rStickX(0.0)
    , m_rStickY(0.0)
    , m_accX(0.0)
    , m_accY(0.0)
    , m_accZ(0.0)
    , m_gyroX(0.0)
    , m_gyroY(0.0)
    , m_gyroZ(0.0)
    , m_angleX(0.0)
    , m_angleY(0.0)
    , m_angleZ(0.0)
    , m_dirXx(0.0)
    , m_dirXy(0.0)
    , m_dirXz(0.0)
    , m_dirYx(0.0)
    , m_dirYy(0.0)
    , m_dirYz(0.0)
    , m_dirZx(0.0)
    , m_dirZy(0.0)
    , m_dirZz(0.0)
{
}

WiiuGamePad*
WiiuGamePad::update()
{
    WKC::WKCWiiuGamePad gp = { };

    WKC::WKCWiiuGamePadUpdate(&gp);

    this->m_isEnabled   = gp.m_isEnabled;
    this->m_isDataValid = gp.m_isDataValid;
    this->m_tpX         = gp.m_tpX;
    this->m_tpY         = gp.m_tpY;
    this->m_tpValidity  = gp.m_tpValidity;
    this->m_tpTouch     = gp.m_tpTouch;
    this->m_contentX    = gp.m_contentX;
    this->m_contentY    = gp.m_contentY;
    this->m_hold        = gp.m_hold;
    this->m_lStickX     = gp.m_lStickX;
    this->m_lStickY     = gp.m_lStickY;
    this->m_rStickX     = gp.m_rStickX;
    this->m_rStickY     = gp.m_rStickY;
    this->m_accX        = gp.m_accX;
    this->m_accY        = gp.m_accY;
    this->m_accZ        = gp.m_accZ;
    this->m_gyroX       = gp.m_gyroX;
    this->m_gyroY       = gp.m_gyroY;
    this->m_gyroZ       = gp.m_gyroZ;
    this->m_angleX      = gp.m_angleX;
    this->m_angleY      = gp.m_angleY;
    this->m_angleZ      = gp.m_angleZ;
    this->m_dirXx       = gp.m_dirXx;
    this->m_dirXy       = gp.m_dirXy;
    this->m_dirXz       = gp.m_dirXz;
    this->m_dirYx       = gp.m_dirYx;
    this->m_dirYy       = gp.m_dirYy;
    this->m_dirYz       = gp.m_dirYz;
    this->m_dirZx       = gp.m_dirZx;
    this->m_dirZy       = gp.m_dirZy;
    this->m_dirZz       = gp.m_dirZz;

    return this;
}

} //namespace
