#pragma once

// Include the real PlatformPath.h
#include_next <WebCore/PlatformPath.h>

// Add WKC platform path types
namespace WebCore {
class PlatformPathImpl;
using PlatformPathPtr = PlatformPathImpl*;
}
