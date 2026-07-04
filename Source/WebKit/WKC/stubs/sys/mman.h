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

// Wii U shim for <sys/mman.h>. devkitPPC/newlib does not ship a memory-mapping
// header, but wtf/FileSystem.cpp includes it under #if !OS(WINDOWS) and
// references mprotect()/msync()/PROT_READ/MS_ASYNC in finalizeMappedFileData().
// The actual mapping primitives live in wtf/posix/FileSystemPOSIX.cpp, which is
// not part of this build, so the map() path is never exercised on Wii U; this
// header only needs to make the referenced names declarable. Found ahead of the
// (absent) system header via the WKC stubs include dir.

#pragma once

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

#define MAP_SHARED  0x01
#define MAP_PRIVATE 0x02
#define MAP_ANON    0x20
#define MAP_ANONYMOUS MAP_ANON
#define MAP_FAILED  ((void*)-1)

#define MS_ASYNC      0x1
#define MS_INVALIDATE 0x2
#define MS_SYNC       0x4

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void* addr, size_t length);
int mprotect(void* addr, size_t length, int prot);
int msync(void* addr, size_t length, int flags);

#ifdef __cplusplus
}
#endif
