#pragma once

// Include the real PlatformImage.h first
#include_next <WebCore/PlatformImage.h>

// Add WKC platform image type
#include "ImageWKC.h"

namespace WebCore {
using PlatformImagePtr = ImageWKC*;
}
