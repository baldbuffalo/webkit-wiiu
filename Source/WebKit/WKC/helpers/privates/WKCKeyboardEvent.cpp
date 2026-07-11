/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCKeyboardEvent.h"
#include "helpers/privates/WKCKeyboardEventPrivate.h"

#include "helpers/privates/WKCEventPrivate.h"

#include "KeyboardEvent.h"
#include "PlatformKeyboardEvent.h"

#include <string.h>

static bool
_platformKeyEvent(WebCore::KeyboardEvent* event, WKC::WKCKeyEvent& ev)
{
    const WebCore::PlatformKeyboardEvent* pe = 0;
    unsigned int mod = 0;
    unsigned int pmod = 0;

    if (!event)
        return false;
    // KeyboardEvent::keyEvent() was renamed to underlyingPlatformEvent().
    pe = event->underlyingPlatformEvent();
    if (!pe)
        return false;

    switch (pe->type()) {
    case WebCore::PlatformKeyboardEvent::Type::KeyDown:
    case WebCore::PlatformKeyboardEvent::Type::RawKeyDown:
        ev.m_type = WKC::EKeyEventPressed;
        ev.m_key = (WKC::Key)event->keyCode();
        break;
    case WebCore::PlatformKeyboardEvent::Type::KeyUp:
        ev.m_type = WKC::EKeyEventReleased;
        ev.m_key = (WKC::Key)event->keyCode();
        break;
    case WebCore::PlatformKeyboardEvent::Type::Char:
        ev.m_type = WKC::EKeyEventChar;
        ev.m_char = event->charCode();
        break;
    default:
        return false;
    }
    // 2026: modifiers() returns OptionSet<PlatformEventModifier> (scoped enum).
    auto mods = pe->modifiers();
    (void)mod;
    pmod = WKC::EModifierNone;
    if (mods.contains(WebCore::PlatformEventModifier::AltKey))
        pmod |= WKC::EModifierAlt;
    if (mods.contains(WebCore::PlatformEventModifier::ControlKey))
        pmod |= WKC::EModifierCtrl;
    if (mods.contains(WebCore::PlatformEventModifier::ShiftKey))
        pmod |= WKC::EModifierShift;
    if (mods.contains(WebCore::PlatformEventModifier::MetaKey))
        pmod |= WKC::EModifierMod1;
    ev.m_modifiers = (WKC::Modifier)pmod;
    return true;
}

namespace WKC {

KeyboardEventPrivate::KeyboardEventPrivate(WebCore::KeyboardEvent* parent)
    : EventPrivate(parent)
{
    ::memset(&m_keyEvent, 0, sizeof(m_keyEvent));
    if (!_platformKeyEvent(parent, m_keyEvent)) {
        ::memset(&m_keyEvent, 0, sizeof(m_keyEvent));
    }
}

KeyboardEventPrivate::~KeyboardEventPrivate()
{
}

KeyboardEvent::KeyboardEvent(KeyboardEventPrivate& parent)
    : Event(parent)
{
}

KeyboardEvent::~KeyboardEvent()
{
}

WKCKeyEvent
KeyboardEvent::keyEvent() const
{
    return reinterpret_cast<KeyboardEventPrivate&>(priv()).keyEvent();
}

} // namespace
