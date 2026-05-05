#pragma once
#include <stdint.h>

// WTF string types must be complete before <optional> is included,
// otherwise GCC 16 SFINAE iterator probing triggers -Wsfinae-incomplete
// warnings when these types are later defined.
#include <wtf/text/WTFString.h>
#include <wtf/text/CString.h>
#include <wtf/text/AtomString.h>

#include "WebCore/StyleTextEdge.h"
#include "WebCore/PlatformPattern.h"
#include "WebCore/PlatformPath.h"
#include "wtf/OwnPtr.h"
