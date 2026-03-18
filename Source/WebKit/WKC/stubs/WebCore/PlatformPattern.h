#pragma once

// Define PlatformPatternPtr for WKC bare metal BEFORE the real header uses it
namespace WebCore {
struct WKCPattern { };
}
using PlatformPatternPtr = WebCore::WKCPattern*;

// Now include the real header — it will skip redefining since CG/Cairo/Skia are not set
#include_next <WebCore/PlatformPattern.h>
