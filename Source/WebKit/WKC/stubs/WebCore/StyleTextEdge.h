#pragma once

#include <stdint.h>

namespace CSS {
namespace Keyword {
struct Auto {};
struct Leading {};
} // namespace Keyword
} // namespace CSS

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

namespace WebCore {
namespace Style {

template<typename DefaultKeyword>
struct TextEdge {
    WebCore::TextEdgeType over  { WebCore::TextEdgeType::Auto };
    WebCore::TextEdgeType under { WebCore::TextEdgeType::Auto };

    bool operator==(const TextEdge&) const = default;
};

} // namespace Style
} // namespace WebCore
