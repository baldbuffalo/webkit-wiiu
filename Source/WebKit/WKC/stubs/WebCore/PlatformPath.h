#pragma once

// WKC platform path types — self-contained stub.
// The real PlatformPath.h contains Cairo-specific code (cairo_path_t etc.)
// that does not compile on WKC bare metal. Path.h only needs PlatformPathPtr,
// which we provide directly here.
namespace WebCore {
class PlatformPathImpl;
using PlatformPathPtr = PlatformPathImpl*;
}
