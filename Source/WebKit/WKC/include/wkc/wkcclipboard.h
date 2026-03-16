#pragma once

#include <wkc/wkcbase.h>

WKC_BEGIN_C_LINKAGE

// Clipboard format types
enum {
    WKC_CLIPBOARD_FORMAT_TEXT       = 0x01,
    WKC_CLIPBOARD_FORMAT_HTML       = 0x02,
    WKC_CLIPBOARD_FORMAT_IMAGE      = 0x04,
    WKC_CLIPBOARD_FORMAT_URI        = 0x08,
    WKC_CLIPBOARD_FORMAT_SMART_PASTE = 0x10,
};

// Image types
enum {
    WKC_IMAGETYPE_ARGB8888       = 0,
    WKC_IMAGETYPE_RGAB5515       = 1,
    WKC_IMAGETYPE_FLAG_HASALPHA  = 0x100,
};

WKC_PEER_API void wkcPasteboardClearPeer(void);
WKC_PEER_API void wkcPasteboardWritePlainTextPeer(const char* text, int length);
WKC_PEER_API void wkcPasteboardWriteURIPeer(const char* url, int urlLen, const char* title, int titleLen);
WKC_PEER_API void wkcPasteboardWriteHTMLPeer(const char* html, int htmlLen, const char* url, int urlLen, const char* plain, int plainLen, bool canSmartCopy);
WKC_PEER_API void wkcPasteboardWriteImagePeer(int type, void* bitmap, int rowbytes, int x, int y, const WKCSize* size);
WKC_PEER_API bool wkcPasteboardIsFormatAvailablePeer(int format);
WKC_PEER_API int  wkcPasteboardReadPlainTextPeer(char* buf, int length);
WKC_PEER_API int  wkcPasteboardReadHTMLPeer(char* buf, int length);
WKC_PEER_API int  wkcPasteboardReadHTMLURIPeer(char* buf, int length);

WKC_END_C_LINKAGE
