#pragma once

#include <stdint.h>

namespace WebCore {
enum class TextEdgeType : uint8_t {
    Auto,
    Text,
    CapHeight,
    ExHeight,
    CJKIdeographic,
    CJKIdeographicInk,
    Leading,
    Alphabetic
};
} // namespace WebCore

#include "WebCore/PlatformPattern.h"
#include "WebCore/PlatformPath.h"
#include "wtf/OwnPtr.h"
