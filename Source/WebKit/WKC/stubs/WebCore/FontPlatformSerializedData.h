#pragma once

#include <wtf/Forward.h>

namespace WebCore {
class SharedBuffer;
}

// Minimal FontPlatformData::CreationData for WKC
// Must match what FontCustomPlatformData uses
struct FontPlatformSerializedData { };

struct CustomFontCreationData {
    WTF::RefPtr<WebCore::SharedBuffer> fontFaceData;
    WTF::String itemInCollection;
};
