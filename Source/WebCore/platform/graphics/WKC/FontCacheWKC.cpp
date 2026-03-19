#include "config.h"
#include "FontCache.h"

#include "Font.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "FontPlatformDataWKC.h"
#include "FontCreationContext.h"
#include <wtf/text/AtomString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

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

std::unique_ptr<FontPlatformData> FontCache::createFontPlatformData(const FontDescription& desc, const AtomString& family, const FontCreationContext&, OptionSet<FontLookupOptions>)
{
    auto pd = makeUnique<FontPlatformData>(
        desc.computedSize(),
        false,
        false,
        desc.orientation(),
        desc.widthVariant(),
        desc.textRenderingMode()
    );

    // Map FontDescription generic family to WKC family constant
    int wkcFamily = WKC_FONT_FAMILY_NONE;
    switch (desc.genericFamily()) {
    case FontDescription::GenericFamilyType::Serif:      wkcFamily = WKC_FONT_FAMILY_SERIF;      break;
    case FontDescription::GenericFamilyType::SansSerif:  wkcFamily = WKC_FONT_FAMILY_SANSSERIF;  break;
    case FontDescription::GenericFamilyType::Monospace:  wkcFamily = WKC_FONT_FAMILY_MONOSPACE;  break;
    case FontDescription::GenericFamilyType::Cursive:    wkcFamily = WKC_FONT_FAMILY_CURSIVE;    break;
    case FontDescription::GenericFamilyType::Fantasy:    wkcFamily = WKC_FONT_FAMILY_FANTASY;    break;
    default: break;
    }

    int w = (int)desc.weight();
    int wkcWeight = WKC_FONT_WEIGHT_NORMAL;
    if      (w < 150) wkcWeight = WKC_FONT_WEIGHT_100;
    else if (w < 250) wkcWeight = WKC_FONT_WEIGHT_200;
    else if (w < 350) wkcWeight = WKC_FONT_WEIGHT_300;
    else if (w < 450) wkcWeight = WKC_FONT_WEIGHT_400;
    else if (w < 550) wkcWeight = WKC_FONT_WEIGHT_500;
    else if (w < 650) wkcWeight = WKC_FONT_WEIGHT_600;
    else if (w < 750) wkcWeight = WKC_FONT_WEIGHT_700;
    else if (w < 850) wkcWeight = WKC_FONT_WEIGHT_800;
    else              wkcWeight = WKC_FONT_WEIGHT_900;

    bool italic = desc.italic() == FontSelectionValue(1);
    bool horizontal = desc.orientation() == FontOrientation::Horizontal;

    auto familyUTF8 = family.string().utf8();
    WKC::WKCFontInfo* info = WKC::WKCFontInfo::create(
        desc.computedSize(), wkcWeight, italic, horizontal, false,
        wkcFamily, familyUTF8.data());

    if (info)
        wkcRegisterFont(pd->hash(), info);

    return pd;
}

Ref<Font> FontCache::lastResortFallbackFont(const FontDescription& fontDescription)
{
    auto data = fontForFamily(fontDescription, "systemfont"_s);
    if (!data)
        data = fontForFamily(fontDescription, "nullfont"_s);
    if (data)
        return *data;
    auto platform = makeUnique<FontPlatformData>(fontDescription.computedSize(), false, false);
    return Font::create(*platform);
}

} // namespace WebCore
