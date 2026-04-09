#pragma once
#include "../../platform/graphics/WKC/PlatformPathWKC.h"
namespace WebCore {
using PlatformPathImpl = PlatformPathWKC;
using PlatformPathPtr = PlatformPathWKC*;
}
// WKC platform path types — self-contained stub.
// The real PlatformPath.h contains Cairo-specific code (cairo_path_t etc.)
// that does not compile on WKC bare metal. Path.h only needs PlatformPathPtr,
// which we provide directly here.
