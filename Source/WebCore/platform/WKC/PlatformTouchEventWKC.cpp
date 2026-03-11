/*
 *  Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"

#if ENABLE(TOUCH_EVENTS)

#include "PlatformTouchEvent.h"
#include "PlatformTouchPoint.h"
#include "WKCPlatformEvents.h"
#include "WKCEnums.h"
#include <wtf/WallTime.h>

namespace WebCore {

PlatformTouchEvent::PlatformTouchEvent(void* event)
{
    WKC::WKCTouchEvent* ev = reinterpret_cast<WKC::WKCTouchEvent*>(event);

    m_timestamp = WallTime::fromRawSeconds(ev->m_timestampinsec);

    switch (ev->m_type) {
    case WKC::ETouchTypeStart:
        m_type = PlatformEvent::Type::TouchStart;  break;
    case WKC::ETouchTypeEnd:
        m_type = PlatformEvent::Type::TouchEnd;    break;
    case WKC::ETouchTypeMove:
        m_type = PlatformEvent::Type::TouchMove;   break;
    case WKC::ETouchTypeCancel:
    default:
        m_type = PlatformEvent::Type::TouchCancel; break;
    }

    for (int i = 0; i < ev->m_npoints; i++)
        m_touchPoints.append(PlatformTouchPoint((void*)&ev->m_points[i]));

    OptionSet<PlatformEvent::Modifier> modifiers;
    if (ev->m_modifiers & WKC::EModifierShift)
        modifiers.add(PlatformEvent::Modifier::ShiftKey);
    if (ev->m_modifiers & WKC::EModifierCtrl)
        modifiers.add(PlatformEvent::Modifier::ControlKey);
    if (ev->m_modifiers & WKC::EModifierAlt)
        modifiers.add(PlatformEvent::Modifier::AltKey);
    if (ev->m_modifiers & WKC::EModifierMod1)
        modifiers.add(PlatformEvent::Modifier::MetaKey);
    m_modifiers = modifiers;
}

PlatformTouchPoint::PlatformTouchPoint(void* obj)
{
    WKC::WKCTouchPoint* pos = reinterpret_cast<WKC::WKCTouchPoint*>(obj);

    m_id = pos->m_id;

    switch (pos->m_state) {
    case WKC::ETouchPointStateReleased:
        m_state = PlatformTouchPoint::State::Released;   break;
    case WKC::ETouchPointStatePressed:
        m_state = PlatformTouchPoint::State::Pressed;    break;
    case WKC::ETouchPointStateMoved:
        m_state = PlatformTouchPoint::State::Moved;      break;
    case WKC::ETouchPointStateStationary:
        m_state = PlatformTouchPoint::State::Stationary; break;
    case WKC::ETouchPointStateCancelled:
    default:
        m_state = PlatformTouchPoint::State::Cancelled;  break;
    }

    m_screenPos = IntPoint(pos->m_pos.fX, pos->m_pos.fY);
    m_pos = m_screenPos;
    m_radiusX = 0;
    m_radiusY = 0;
    m_rotationAngle = 0.0f;
    m_force = 0.0f;
}

} // namespace WebCore

#endif // ENABLE(TOUCH_EVENTS)
