#pragma once
#include <WebCore/FontPlatformSerializedData.h>
#include_next <WebCore/FontPlatformData.h>

namespace WKC { class WKCFontInfo; }
namespace WebCore {
    // WKC accessor — defined in FontPlatformDataWKC.cpp
    // Declared here so WKC graphics code can call platformData.font()
}
