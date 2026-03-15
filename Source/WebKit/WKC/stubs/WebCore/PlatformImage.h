#pragma once

// WKC platform image type — replaces generated PlatformImage.h for this platform
namespace WebCore {
class ImageWKC;
using PlatformImagePtr = ImageWKC*;
}
