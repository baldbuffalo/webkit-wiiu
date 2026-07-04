#pragma once
// Compatibility shim for the 2014-era WKC glue.
//
// WTF::AtomicString was renamed to WTF::AtomString (header AtomString.h) and the
// old "AtomicString.h" was deleted. Resolve the stale include and alias the old
// name so the glue's `WTF::AtomicString` usages keep compiling. This only touches
// files that still include the old header (WKC glue); upstream WebCore already
// uses AtomString directly.
#include <wtf/text/AtomString.h>

namespace WTF {
using AtomicString = AtomString;
}
