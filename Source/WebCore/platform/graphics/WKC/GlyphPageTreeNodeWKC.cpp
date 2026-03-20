#include "config.h"
#include "GlyphPage.h"
#include "Font.h"

namespace WebCore {

bool GlyphPage::fill(std::span<const char16_t> characterBuffer)
{
    if (!m_font)
        return false;

    bool haveGlyphs = false;
    for (size_t i = 0; i < characterBuffer.size(); i++) {
        Glyph glyph = characterBuffer[i];
        if (glyph) {
            setGlyphForIndex(i, glyph, ColorGlyphType::Outline);
            haveGlyphs = true;
        }
    }
    return haveGlyphs;
}

} // namespace WebCore
