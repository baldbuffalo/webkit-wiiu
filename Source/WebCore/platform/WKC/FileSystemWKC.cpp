/*
 * Copyright (C) 2007, 2009 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora, Ltd.
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include <wtf/FileSystem.h>
#include <wtf/WallTime.h>
#include <wtf/text/CString.h>
#include "NotImplemented.h"

#ifdef __WKC_IMPLICIT_INCLUDE_SYSSTAT
#include <sys/stat.h>
#endif

namespace FileSystem {

bool fileExists(const String& path)
{
    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return false;
    wkcFileFClosePeer(fp);
    return true;
}

bool deleteFile(const String& path)
{
    wkcFileUnlinkPeer(path.utf8().data());
    return true;
}

bool deleteEmptyDirectory(const String& path)
{
    notImplemented();
    return false;
}

std::optional<uint64_t> fileSize(const String& path)
{
    if (path.isEmpty())
        return std::nullopt;

    struct stat st;
    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return std::nullopt;

    int err = wkcFileFStatPeer(fp, &st);
    wkcFileFClosePeer(fp);
    if (err == -1)
        return std::nullopt;

    return static_cast<uint64_t>(st.st_size);
}

std::optional<WallTime> fileModificationTime(const String& path)
{
    if (path.isEmpty())
        return std::nullopt;

    struct stat st;
    void* fp = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "rb");
    if (!fp)
        return std::nullopt;

    int err = wkcFileFStatPeer(fp, &st);
    wkcFileFClosePeer(fp);
    if (err == -1)
        return std::nullopt;

    return WallTime::fromRawSeconds(st.st_mtime);
}

String pathByAppendingComponent(const String& path, const String& component)
{
    char buf[MAX_PATH] = {0};
    int ret = wkcFilePathByAppendingComponentPeer(path.utf8().data(), component.utf8().data(), buf, MAX_PATH);
    if (!ret)
        return String();
    return String::fromUTF8(buf);
}

bool makeAllDirectories(const String& path)
{
    return wkcFileMakeAllDirectoriesPeer(path.utf8().data());
}

String homeDirectoryPath()
{
    char buf[MAX_PATH] = {0};
    int ret = wkcFileHomeDirectoryPathPeer(buf, MAX_PATH);
    if (!ret)
        return String();
    return String::fromUTF8(buf);
}

String pathFileName(const String& pathName)
{
    char buf[MAX_PATH] = {0};
    if (pathName.isEmpty())
        return String();
    int ret = wkcFilePathGetFileNamePeer(pathName.utf8().data(), buf, MAX_PATH);
    if (!ret)
        return String();
    return String::fromUTF8(buf);
}

String directoryName(const String& path)
{
    char buf[MAX_PATH] = {0};
    int ret = wkcFileDirectoryNamePeer(path.utf8().data(), buf, MAX_PATH);
    if (!ret)
        return String();
    return String::fromUTF8(buf);
}

Vector<String> listDirectory(const String& path)
{
    Vector<String> entries;
    char name[MAX_PATH];
    char fullpath[MAX_PATH];

    void* dir = wkcFileOpenDirectoryPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), "*");
    if (!dir)
        return entries;

    while (0 == wkcFileReadDirectoryPeer(dir, name, MAX_PATH)) {
        int ret = wkcFilePathByAppendingComponentPeer(path.utf8().data(), name, fullpath, MAX_PATH);
        if (!ret)
            break;
        entries.append(String::fromUTF8(fullpath));
    }

    wkcFileCloseDirectoryPeer(dir);
    return entries;
}

std::pair<String, PlatformFileHandle> openTemporaryFile(StringView prefix, StringView suffix)
{
    char name[1024] = {0};
    void* fd = wkcFileOpenTemporaryFilePeer(prefix.utf8().data(), name, sizeof(name));
    if (!fd)
        return { String(), invalidPlatformFileHandle };
    return { String::fromUTF8(name), (PlatformFileHandle)reinterpret_cast<uintptr_t>(fd) };
}

PlatformFileHandle openFile(const String& path, FileOpenMode mode)
{
    const char* modeStr = (mode == FileOpenMode::Read) ? "r" : "w";
    void* fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), modeStr);
    if (!fd)
        return invalidPlatformFileHandle;
    return (PlatformFileHandle)reinterpret_cast<uintptr_t>(fd);
}

void closeFile(PlatformFileHandle& handle)
{
    if (handle == invalidPlatformFileHandle)
        return;
    void* fd = reinterpret_cast<void*>(handle);
    wkcFileFClosePeer(fd);
    handle = invalidPlatformFileHandle;
}

int writeToFile(PlatformFileHandle handle, const void* data, int length)
{
    if (handle == invalidPlatformFileHandle)
        return 0;
    void* fd = reinterpret_cast<void*>(handle);
    return (int)wkcFileFWritePeer(data, 1, length, fd);
}

int readFromFile(PlatformFileHandle handle, void* data, int length)
{
    if (handle == invalidPlatformFileHandle)
        return 0;
    void* fd = reinterpret_cast<void*>(handle);
    return (int)wkcFileFReadPeer(data, 1, length, fd);
}

long long seekFile(PlatformFileHandle handle, long long offset, FileSeekOrigin origin)
{
    if (handle == invalidPlatformFileHandle)
        return -1;
    void* fd = reinterpret_cast<void*>(handle);
    return (long long)wkcFileFSeekPeer(fd, offset, (int)origin);
}

} // namespace FileSystem
