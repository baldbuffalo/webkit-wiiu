#pragma once

// TextEdgeType enum used by StyleTextEdge.h — must be declared before
// any upstream CSS style headers are included.
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
