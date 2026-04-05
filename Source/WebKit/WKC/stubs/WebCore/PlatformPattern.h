#pragma once

// WKC platform pattern type.
// The real PlatformPattern.h contains Cairo/CoreGraphics-specific code
// that does not apply to WKC bare metal. This stub provides only what
// Pattern.h needs: PlatformPatternPtr.
namespace WebCore {
class PlatformPatternWKC;
using PlatformPatternPtr = PlatformPatternWKC*;
}
