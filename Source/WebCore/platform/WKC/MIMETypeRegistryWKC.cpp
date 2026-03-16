#include "config.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"

namespace WebCore {

struct ExtensionMap {
    const char* extension;
    const char* mimeType;
};

static const ExtensionMap extensionMap[] = {
    { "bmp",   "image/bmp" },
    { "css",   "text/css" },
    { "gif",   "image/gif" },
    { "htm",   "text/html" },
    { "html",  "text/html" },
    { "ico",   "image/x-icon" },
    { "jpeg",  "image/jpeg" },
    { "jpg",   "image/jpeg" },
    { "js",    "text/javascript" },
    { "json",  "application/json" },
    { "pdf",   "application/pdf" },
    { "png",   "image/png" },
    { "rss",   "application/rss+xml" },
    { "svg",   "image/svg+xml" },
    { "text",  "text/plain" },
    { "txt",   "text/plain" },
    { "webp",  "image/webp" },
    { "woff",  "font/woff" },
    { "woff2", "font/woff2" },
    { "xml",   "text/xml" },
    { "xsl",   "text/xsl" },
    { "xhtml", "application/xhtml+xml" },
    { "wml",   "text/vnd.wap.wml" },
    { "wmlc",  "application/vnd.wap.wmlc" },
    { "mht",   "multipart/related" },
    { "mhtml", "multipart/related" },
    { "mpo",   "image/mpo" },
    { nullptr, nullptr }
};

String MIMETypeRegistry::mimeTypeForExtension(StringView ext)
{
    String s = ext.convertToASCIILowercase();
    for (const ExtensionMap* e = extensionMap; e->extension; ++e) {
        if (s == e->extension)
            return String::fromLatin1(e->mimeType);
    }
    return String();
}

bool MIMETypeRegistry::isApplicationPluginMIMEType(const String&)
{
    return false;
}

} // namespace WebCore
