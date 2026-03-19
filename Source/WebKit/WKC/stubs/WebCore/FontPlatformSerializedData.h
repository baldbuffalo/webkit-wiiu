#pragma once

#include <wtf/Forward.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore { class SharedBuffer; }

struct FontPlatformSerializedData { };

struct CustomFontCreationData {
    WTF::RefPtr<WebCore::SharedBuffer> fontFaceData;
    WTF::String itemInCollection;
};
