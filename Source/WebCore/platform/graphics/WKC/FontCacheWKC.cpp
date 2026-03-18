#include "config.h"
#include "FontCache.h"

#include "Font.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "FontCreationContext.h"
#include <wtf/text/AtomString.h>

namespace WebCore {

void FontCache::platformInit()
{
}

Vector<String> FontCache::systemFontFamilies()
{
    return { };
}

bool FontCache::isSystemFontForbiddenForEditing(const String&)
{
    return false;
}

ASCIILiteral FontCache::platformAlternateFamilyName(const String&)
{
    return { };
}

std::unique_ptr<FontPlatformData> FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomString&, const FontCreationContext&, OptionSet<FontLookupOptions>)
{
    auto ret = makeUnique<FontPlatformData>(
        fontDescription.computedSize(),
        fontDescription.syntheticBold(),
        fontDescription.syntheticOblique(),
        fontDescription.orientation(),
        fontDescription.widthVariant(),
        fontDescription.textRenderingMode()
    );
    return ret;
}

Ref<Font> FontCache::lastResortFallbackFont(const FontDescription& fontDescription)
{
    auto data = fontForFamily(fontDescription, "systemfont"_s);
    if (!data)
        data = fontForFamily(fontDescription, "nullfont"_s);
    if (data)
        return *data;
    // Last resort — return an empty font
    auto platform = makeUnique<FontPlatformData>(fontDescription.computedSize(), false, false);
    return Font::create(*platform);
}

} // namespace WebCore
