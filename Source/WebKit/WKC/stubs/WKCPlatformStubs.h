#pragma once

// Force-included umbrella for all WKC platform type stubs.
// Ensures PlatformPatternPtr, PlatformPathPtr, and PlatformImagePtr
// are defined before any WebCore header pulls in platform-specific graphics headers.

#include "WebCore/PlatformPattern.h"
#include "WebCore/PlatformPath.h"
#include "WebCore/PlatformImage.h"
#include "wtf/OwnPtr.h"
