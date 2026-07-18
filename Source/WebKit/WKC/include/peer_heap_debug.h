/*
 *  peer_heap_debug.h
 *
 *  WKC heap-diagnostics peer interface.
 *
 *  This is the engine<->platform contract for the optional heap memory-map and
 *  leak-tracking diagnostics used by WKC::Heap (webkit/WKCMemoryInfo.cpp). The
 *  engine declares and calls these; the platform integration (wave-browser's
 *  peer layer) provides the implementations against the target's allocator.
 *
 *  The *WKC structs are the C-ABI mirror of the public WKC::Heap structs in
 *  WKCMemoryInfo.h (which is included before this header in WKCMemoryInfo.cpp),
 *  so their layouts are kept identical field-for-field and the size assertions
 *  in WKCMemoryInfo.cpp hold by construction.
 */

#ifndef PEER_HEAP_DEBUG_H
#define PEER_HEAP_DEBUG_H

#include <stddef.h>
#include <time.h>

// Mirror of WKC::Heap kMaxFileNameLength-sized name buffers. WKCMemoryInfo.h,
// which defines kMaxFileNameLength, is included ahead of this header.
#ifndef kMaxNameLengthWKC
#define kMaxNameLengthWKC (kMaxFileNameLength + 1)
#endif

// --- C-ABI mirrors of the public WKC::Heap structures ---

struct UsedMemoryInfoWKC_ {
    void* adr;
    unsigned int requestSize;
    unsigned int usedSize;
    unsigned short classID;
    bool outOfRange;
};
typedef struct UsedMemoryInfoWKC_ UsedMemoryInfoWKC;

struct SpanInfoWKC_ {
    void* span;
    void* head;
    void* tail;
    void* nextAddress;
    bool used;
    unsigned short pages;
    unsigned short classID;
    unsigned int blockSize;
    unsigned int usedBlocks;
    unsigned int maxBlocks;
    unsigned int size;
    unsigned int requestedSize;
    unsigned int numUsedMemArray;
    UsedMemoryInfoWKC* usedMemArray;
    UsedMemoryInfoWKC** usedMemPtrArray;
};
typedef struct SpanInfoWKC_ SpanInfoWKC;

struct MemoryInfoWKC_ {
    unsigned int pageSize;
    unsigned int numSpanArray;
    SpanInfoWKC* spanArray;
    SpanInfoWKC** spanPtrArray;
};
typedef struct MemoryInfoWKC_ MemoryInfoWKC;

struct StackObjectInfoWKC_ {
    void* adr;
    unsigned int line;
    unsigned int displacement;
    unsigned char name[kMaxNameLengthWKC];
};
typedef struct StackObjectInfoWKC_ StackObjectInfoWKC;

typedef struct StackTraceInfoWKC_ StackTraceInfoWKC;
struct StackTraceInfoWKC_ {
    StackObjectInfoWKC* obj;
    StackTraceInfoWKC* next;
    StackTraceInfoWKC* prev;
};

typedef struct MemoryLeakInfoWKC_ MemoryLeakInfoWKC;
struct MemoryLeakInfoWKC_ {
    void* adr;
    unsigned int size;
    unsigned int reqSize;
    time_t curTime;
    MemoryLeakInfoWKC* next;
    MemoryLeakInfoWKC* prev;
};

typedef struct MemoryLeakNodeWKC_ MemoryLeakNodeWKC;
struct MemoryLeakNodeWKC_ {
    unsigned int numInfo;
    MemoryLeakInfoWKC* memHead;
    MemoryLeakInfoWKC* memTail;
    unsigned int numTrace;
    StackTraceInfoWKC* stHead;
    StackTraceInfoWKC* stTail;
    MemoryLeakNodeWKC* next;
    MemoryLeakNodeWKC* prev;
};

struct MemoryLeakRootWKC_ {
    MemoryLeakNodeWKC* head;
    MemoryLeakNodeWKC* tail;
    unsigned int num;
    unsigned int leakReqSum;
    unsigned int leakSum;
};
typedef struct MemoryLeakRootWKC_ MemoryLeakRootWKC;

typedef void (*WKCMemoryLeakDumpProc)(void* in_ctx);

// --- Peer entry points (implemented by the platform integration) ---

bool wkcHeapDebugEnableMemoryMapPeer(bool in_set);
void wkcHeapDebugGetMemoryMapPeer(MemoryInfoWKC& memInfo, bool needUsedMemory);
bool wkcHeapDebugAllocMemoryMapPeer(MemoryInfoWKC& memInfo);
void wkcHeapDebugFreeMemoryMapPeer(MemoryInfoWKC& memInfo);
void wkcHeapDebugSetMemoryLeakDumpProcPeer(WKCMemoryLeakDumpProc in_proc, void* in_ctx);
bool wkcHeapDebugGetMemoryLeakInfoPeer(MemoryLeakRootWKC& leakRoot, bool resolveSymbol);
void wkcHeapDebugClearStackTracePeer(void);
void wkcHeapDebugClearMemoryLeakInfoPeer(MemoryLeakRootWKC& leakRoot);

#endif // PEER_HEAP_DEBUG_H
