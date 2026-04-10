#pragma once

#include <stdint.h>
#include <optional>
#include <variant>

// Minimal CSS keyword tags needed by StyleLineFitEdge and StyleTextBoxEdge
namespace CSS {
namespace Keyword {
struct Auto  {};
struct Leading {};
} // namespace Keyword
} // namespace CSS

namespace WebCore {

// TextEdgeType, TextEdgeOver, TextEdgeUnder are defined in RenderStyleConstants.h
// Only define TextEdgeType here since it's needed before RenderStyleConstants.h loads
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

// Rendering-side flat TextEdge struct (replaces rendering/style/StyleTextEdge.h)
struct TextEdge {
    TextEdgeType over  { TextEdgeType::Auto };
    TextEdgeType under { TextEdgeType::Auto };
    bool operator==(const TextEdge&) const = default;
};

namespace Style {

struct TextEdgePair {
    // Uses TextEdgeOver / TextEdgeUnder from RenderStyleConstants.h
    // Forward declared here as uint8_t placeholders to avoid circular dependency
    uint8_t over  { 0 };
    uint8_t under { 0 };
    bool operator==(const TextEdgePair&) const = default;
};

// Style-side TextEdge template (replaces style/values/inline/StyleTextEdge.h)
template<typename K>
struct TextEdge {
    using Keyword = K;

    TextEdge() : m_value(Keyword{}) { }
    TextEdge(Keyword kw) : m_value(kw) { }

    bool isKeyword() const { return std::holds_alternative<Keyword>(m_value); }
    bool isTextEdgePair() const { return std::holds_alternative<TextEdgePair>(m_value); }

    std::optional<TextEdgePair> tryTextEdgePair() const
    {
        if (isTextEdgePair())
            return std::get<TextEdgePair>(m_value);
        return std::nullopt;
    }

    bool operator==(const TextEdge<K>&) const = default;

protected:
    std::variant<Keyword, TextEdgePair> m_value;
};

} // namespace Style
} // namespace WebCore
