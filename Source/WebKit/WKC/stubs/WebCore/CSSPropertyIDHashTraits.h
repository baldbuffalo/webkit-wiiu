#pragma once
#include <wtf/HashFunctions.h>
#include <wtf/HashTraits.h>
#include <limits>
#include "CSSPropertyNames.h"

namespace WTF {
template<> struct DefaultHash<WebCore::CSSPropertyID> : IntHash<unsigned> { };
template<> struct HashTraits<WebCore::CSSPropertyID> : GenericHashTraits<WebCore::CSSPropertyID> {
    static const bool emptyValueIsZero = true;
    static void constructDeletedValue(WebCore::CSSPropertyID& slot) { slot = static_cast<WebCore::CSSPropertyID>(std::numeric_limits<uint16_t>::max()); }
    static bool isDeletedValue(WebCore::CSSPropertyID value) { return static_cast<uint16_t>(value) == std::numeric_limits<uint16_t>::max(); }
};
} // namespace WTF
