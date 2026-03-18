#include "config.h"
#include "PlatformWheelEvent.h"

#include <wkc/wkcbase.h>
#include "WKCPlatformEvents.h"
#include "Scrollbar.h"
#include <wtf/WallTime.h>

namespace WebCore {

PlatformWheelEvent wkcCreateWheelEvent(void* event)
{
    WKC::WKCWheelEvent* ev = static_cast<WKC::WKCWheelEvent*>(event);

    float wheelTicksX = ev->m_dx > 0 ? 1.0f : ev->m_dx < 0 ? -1.0f : 0.0f;
    float wheelTicksY = ev->m_dy > 0 ? 1.0f : ev->m_dy < 0 ? -1.0f : 0.0f;
    float deltaX = wheelTicksX * static_cast<float>(Scrollbar::pixelsPerLineStep());
    float deltaY = wheelTicksY * static_cast<float>(Scrollbar::pixelsPerLineStep());

    OptionSet<PlatformEvent::Modifier> modifiers;
    if (ev->m_modifiers & WKC::EModifierShift)
        modifiers.add(PlatformEvent::Modifier::ShiftKey);
    if (ev->m_modifiers & WKC::EModifierCtrl)
        modifiers.add(PlatformEvent::Modifier::ControlKey);
    if (ev->m_modifiers & WKC::EModifierAlt)
        modifiers.add(PlatformEvent::Modifier::AltKey);
    if (ev->m_modifiers & WKC::EModifierMod1)
        modifiers.add(PlatformEvent::Modifier::MetaKey);

    return PlatformWheelEvent(
        IntPoint(ev->m_x, ev->m_y),
        IntPoint(ev->m_x, ev->m_y),
        deltaX, deltaY,
        wheelTicksX, wheelTicksY,
        PlatformWheelEventGranularity::ScrollByPixelWheelEvent,
        modifiers.contains(PlatformEvent::Modifier::ShiftKey),
        modifiers.contains(PlatformEvent::Modifier::ControlKey),
        modifiers.contains(PlatformEvent::Modifier::AltKey),
        modifiers.contains(PlatformEvent::Modifier::MetaKey)
    );
}

} // namespace WebCore
