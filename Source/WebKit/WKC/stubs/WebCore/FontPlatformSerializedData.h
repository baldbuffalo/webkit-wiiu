#pragma once

namespace WKC { class WKCFontInfo; }

struct FontPlatformSerializedData { };

struct CustomFontCreationData {
    WKC::WKCFontInfo* font { nullptr };
};
