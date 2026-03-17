#pragma once

// Include the real PlatformPattern.h first
#include_next <WebCore/PlatformPattern.h>

// Add WKC platform pattern type
namespace WebCore {
struct WKCPattern { };
}
using PlatformPatternPtr = WebCore::WKCPattern*;
