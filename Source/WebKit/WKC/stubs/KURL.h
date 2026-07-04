#pragma once
// Compatibility shim for the 2014-era WKC glue.
//
// The WKC port was written when WebCore's URL class was named KURL and lived in
// "KURL.h". Modern WebKit renamed it to WTF::URL (in <wtf/URL.h>) and deleted
// KURL.h. Rather than rename KURL -> URL across ~40 glue files (and fight the
// fact that WebCore::URL is not universally in scope), this shim resolves every
// stale `#include "KURL.h"` and aliases the old name to the new type, so all the
// `WebCore::KURL` / bare `KURL` type usages keep compiling unchanged.
//
// NOTE: this only aliases the *type*. Code that used removed URL APIs
// (e.g. WebCore::ParsedURLString) still needs modernizing at the call site.
#include <wtf/URL.h>

namespace WebCore {
using KURL = WTF::URL;
}
