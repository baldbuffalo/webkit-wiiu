#include "config.h"
#include <wtf/FileSystem.h>
#include <wtf/WallTime.h>
#include <wtf/text/CString.h>
#include "NotImplemented.h"
#include <wkc/wkcpeer.h>
#include <sys/stat.h>

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

namespace WTF::FileSystemImpl {

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

std::pair<String, FileHandle> openTemporaryFile(StringView prefix, StringView suffix)
{
    char name[1024] = {0};
    void* fd = wkcFileOpenTemporaryFilePeer(prefix.utf8().data(), name, sizeof(name));
    if (!fd)
        return { String(), FileHandle() };
    return { String::fromUTF8(name), FileHandle(reinterpret_cast<uintptr_t>(fd)) };
}

FileHandle openFile(const String& path, FileOpenMode mode)
{
    const char* modeStr = (mode == FileOpenMode::Read) ? "r" : "w";
    void* fd = wkcFileFOpenPeer(WKC_FILEOPEN_USAGE_WEBCORE, path.utf8().data(), modeStr);
    if (!fd)
        return FileHandle();
    return FileHandle(reinterpret_cast<uintptr_t>(fd));
}

void closeFile(FileHandle& handle)
{
    if (!handle)
        return;
    void* fd = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    wkcFileFClosePeer(fd);
    handle = FileHandle();
}

int writeToFile(FileHandle handle, const void* data, int length)
{
    if (!handle)
        return 0;
    void* fd = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    return (int)wkcFileFWritePeer(data, 1, length, fd);
}

int readFromFile(FileHandle handle, void* data, int length)
{
    if (!handle)
        return 0;
    void* fd = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    return (int)wkcFileFReadPeer(data, 1, length, fd);
}

long long seekFile(FileHandle handle, long long offset, FileSeekOrigin origin)
{
    if (!handle)
        return -1;
    void* fd = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    return (long long)wkcFileFSeekPeer(fd, offset, (int)origin);
}

} // namespace WTF::FileSystemImpl
