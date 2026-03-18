#include "config.h"
#include "FontCache.h"

#include "Font.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "FontPlatformDataWKC.h"
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

std::unique_ptr<FontPlatformData> FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomString& family, const FontCreationContext&, OptionSet<FontLookupOptions>)
{
    auto ret = makeUnique<FontPlatformData>(fontDescription, family);
    if (!ret->font() || (!ret->font()->font() && family != "nullfont"_s))
        return nullptr;
    return ret;
}

Ref<Font> FontCache::lastResortFallbackFont(const FontDescription& fontDescription)
{
    if (auto data = fontForFamily(fontDescription, "systemfont"_s))
        return *data;
    return *fontForFamily(fontDescription, "nullfont"_s);
}

} // namespace WebCore
