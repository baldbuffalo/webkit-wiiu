/*
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * All rights reserved.
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PlatformMouseEvent.h"
#include "WKCPlatformEvents.h"
#include <wtf/WallTime.h>

namespace WebCore {

PlatformMouseEvent::PlatformMouseEvent(void* event)
{
    WKC::WKCMouseEvent* ev = (WKC::WKCMouseEvent*)event;

    m_timestamp = WallTime::fromRawSeconds(ev->m_timestampinsec);
    m_position = IntPoint(ev->m_x, ev->m_y);
    m_globalPosition = IntPoint(ev->m_x, ev->m_y);
    m_clickCount = 1;

    switch (ev->m_type) {
    case WKC::EMouseEventDown:
    case WKC::EMouseEventDoubleClick:
    case WKC::EMouseEventLongPressed:
        m_type = PlatformEvent::Type::MousePressed;
        break;
    case WKC::EMouseEventUp:
        m_type = PlatformEvent::Type::MouseReleased;
        break;
    case WKC::EMouseEventMove:
    case WKC::EMouseEventDrag:
        m_type = PlatformEvent::Type::MouseMoved;
        break;
    default:
        m_type = PlatformEvent::Type::MousePressed;
    }

    if (ev->m_type == WKC::EMouseEventDoubleClick)
        m_clickCount = 2;

    m_button = MouseButton::None;
    switch (ev->m_button) {
    case WKC::EMouseButtonNone:
        m_button = MouseButton::None;   break;
    case WKC::EMouseButtonLeft:
        m_button = MouseButton::Left;   break;
    case WKC::EMouseButtonMiddle:
        m_button = MouseButton::Middle; break;
    case WKC::EMouseButtonRight:
        m_button = MouseButton::Right;  break;
    default:
        break;
    }

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

} // namespace WebCore
