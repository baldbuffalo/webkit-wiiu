#include "config.h"
#include "PlatformMouseEvent.h"
#include <wkc/wkcbase.h>
#include "WKCPlatformEvents.h"
#include <wtf/WallTime.h>

namespace WebCore {

PlatformMouseEvent wkcCreateMouseEvent(void* event)
{
    WKC::WKCMouseEvent* ev = static_cast<WKC::WKCMouseEvent*>(event);

    PlatformEvent::Type type = PlatformEvent::Type::MousePressed;
    switch (ev->m_type) {
    case WKC::EMouseEventDown:
    case WKC::EMouseEventDoubleClick:
    case WKC::EMouseEventLongPressed:
        type = PlatformEvent::Type::MousePressed;
        break;
    case WKC::EMouseEventUp:
        type = PlatformEvent::Type::MouseReleased;
        break;
    case WKC::EMouseEventMove:
    case WKC::EMouseEventDrag:
        type = PlatformEvent::Type::MouseMoved;
        break;
    default:
        break;
    }

    int clickCount = (ev->m_type == WKC::EMouseEventDoubleClick) ? 2 : 1;

    MouseButton button = MouseButton::None;
    switch (ev->m_button) {
    case WKC::EMouseButtonLeft:   button = MouseButton::Left;   break;
    case WKC::EMouseButtonMiddle: button = MouseButton::Middle; break;
    case WKC::EMouseButtonRight:  button = MouseButton::Right;  break;
    default: break;
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

    DoublePoint position(ev->m_x, ev->m_y);

    return PlatformMouseEvent(
        position, position,
        button, type, clickCount,
        modifiers,
        MonotonicTime::fromRawSeconds(ev->m_timestampinsec),
        0.0,
        SyntheticClickType::NoTap,
        MouseEventInputSource::UserDriven
    );
}

} // namespace WebCore
