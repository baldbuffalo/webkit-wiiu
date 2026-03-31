#include "config.h"
#include "HTMLMeterElement.h"
#include "HTMLNames.h"
#include <wtf/TypeCasts.h>

namespace WTF {

bool TypeCastTraits<const WebCore::HTMLMeterElement, const WebCore::Element, false>::isOfType(const WebCore::Element& e)
{
    return e.hasTagName(WebCore::HTMLNames::meterTag);
}

} // namespace WTF
