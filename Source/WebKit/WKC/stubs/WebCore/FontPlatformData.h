#pragma once
// Define IPC stub types before the real FontPlatformData.h uses them
namespace WebCore {
struct FontPlatformSerializedData { };
struct CustomFontCreationData { };
}
#include_next <WebCore/FontPlatformData.h>
