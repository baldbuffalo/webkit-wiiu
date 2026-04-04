#pragma once

// Pull in the real PlatformPattern.h from platform/graphics/
#include_next <WebCore/PlatformPattern.h>

// Add WKC platform pattern type
namespace WebCore {
class PlatformPatternWKC;
using PlatformPatternPtr = PlatformPatternWKC*;
}
