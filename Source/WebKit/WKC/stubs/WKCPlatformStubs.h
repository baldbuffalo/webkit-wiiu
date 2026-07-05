#pragma once
#include <stdint.h>

// --- Wii U platform capability macros ---
// Force-included ahead of config.h/PlatformHave.h, so these win over the
// defaults there. Upstream only defines HAVE_LOCALTIME_R under OS(DARWIN),
// leaving it undefined on devkitPPC and driving wtf/DateMath.cpp and
// wtf/GregorianDateTime.cpp into the localtime_s() branch, which newlib does
// not provide. devkitPPC's newlib has the POSIX localtime_r(), so select it.
#if !defined(HAVE_LOCALTIME_R)
#define HAVE_LOCALTIME_R 1
#endif

// We provide the mmap surface for Wii U (see stubs/sys/mman.h + MmapSpan uses
// mmap/munmap/mprotect/msync). Declaring HAVE(MMAP) makes MappedFileData use its
// MmapSpan-backed branch -- the one that actually defines span()/mutableSpan() --
// instead of leaving the class an empty shell, which is what broke
// wtf/FileSystem.cpp (MappedFileData has no member 'mutableSpan').
#if !defined(HAVE_MMAP)
#define HAVE_MMAP 1
#endif

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
