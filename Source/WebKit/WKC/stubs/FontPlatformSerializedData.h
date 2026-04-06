#pragma once

// WKC stub for font IPC serialization.
// WKC runs single-process, fonts are never serialized over IPC.
namespace WebCore {

struct FontPlatformSerializedData {
};

struct CustomFontCreationData {
};

} // namespace WebCore
