/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

// Wii U shim for <bmalloc/pas_process.h>. libpas (bmalloc's page allocator) is
// not built for this port -- BENABLE(LIBPAS) is 0 on 32-bit PowerPC because it
// requires BCPU(ADDRESS64) -- so the real header is absent. wtf/Threading.cpp
// still includes it unconditionally and calls pas_process_is_shutting_down();
// the process never enters libpas shutdown here, so the answer is always false.
// This header is found ahead of Source/bmalloc via the WKC stubs include dir.

#pragma once

#include <stdbool.h>

static inline bool pas_process_is_shutting_down(void)
{
    return false;
}
