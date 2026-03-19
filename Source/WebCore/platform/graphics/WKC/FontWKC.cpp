#include "config.h"
#include "Font.h"
#include "FontPlatformDataWKC.h"
#include "GlyphBuffer.h"
#include "GraphicsContext.h"
#include "TextRun.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

int wkcGetTextWidth(void* in_font, int in_flags, const unsigned short* in_str, int in_len, int* out_clip_width)
{
    return wkcFontGetTextWidthPeer(in_font, in_flags, in_str, in_len, out_clip_width);
}

} // namespace WebCore
