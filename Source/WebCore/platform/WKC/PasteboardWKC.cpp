#include "config.h"
#include "Pasteboard.h"

#include "DocumentFragment.h"
#include "Image.h"
#include "ImageWKC.h"
#include "NotImplemented.h"
#include "PasteboardCustomData.h"
#include "SerializedAttachmentData.h"
#include <wtf/URL.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

std::unique_ptr<Pasteboard> Pasteboard::createForCopyAndPaste(std::unique_ptr<PasteboardContext>&& context)
{
    return makeUnique<Pasteboard>(WTFMove(context));
}

Pasteboard::Pasteboard(std::unique_ptr<PasteboardContext>&& context)
    : m_context(WTFMove(context))
{
}

void Pasteboard::clear()
{
    wkcPasteboardClearPeer();
}

void Pasteboard::writePlainText(const String& text, SmartReplaceOption)
{
    clear();
    auto utf8 = text.utf8();
    wkcPasteboardWritePlainTextPeer(utf8.data(), (int)utf8.length());
}

void Pasteboard::write(const PasteboardURL& pasteboardURL)
{
    clear();
    auto urlStr = pasteboardURL.url.string().utf8();
    auto titleStr = pasteboardURL.title.utf8();
    wkcPasteboardWriteURIPeer(urlStr.data(), (int)urlStr.length(),
                               titleStr.data(), (int)titleStr.length());
}

void Pasteboard::write(const PasteboardWebContent& content)
{
    notImplemented();
}

void Pasteboard::write(const PasteboardImage& pasteboardImage)
{
    clear();
    Image* image = pasteboardImage.image.get();
    if (!image)
        return;

    auto nativeImage = image->nativeImage();
    ImageWKC* bitmap = nativeImage ? reinterpret_cast<ImageWKC*>(nativeImage.get()) : nullptr;
    if (!bitmap)
        return;

    int type = WKC_IMAGETYPE_ARGB8888;
    switch (bitmap->type()) {
    case ImageWKC::EColorARGB8888:
        type = WKC_IMAGETYPE_ARGB8888;
        break;
    case ImageWKC::EColorRGB565:
        type = WKC_IMAGETYPE_RGAB5515;
        break;
    default:
        return;
    }
    if (bitmap->hasAlpha())
        type |= WKC_IMAGETYPE_FLAG_HASALPHA;

    const WKCSize size = { bitmap->size().width(), bitmap->size().height() };
    wkcPasteboardWriteImagePeer(type, bitmap->bitmap(), bitmap->rowbytes(), nullptr, 0, &size);
}

void Pasteboard::writeCustomData(const Vector<PasteboardCustomData>&)
{
    notImplemented();
}

bool Pasteboard::canSmartReplace()
{
    return wkcPasteboardIsFormatAvailablePeer(WKC_CLIPBOARD_FORMAT_SMART_PASTE);
}

void Pasteboard::read(PasteboardPlainText& text, PlainTextURLReadingPolicy, std::optional<size_t>)
{
    int len = wkcPasteboardReadPlainTextPeer(nullptr, 0);
    if (len <= 0)
        return;
    Vector<char> buf(len + 1);
    wkcPasteboardReadPlainTextPeer(buf.mutableSpan().data(), len + 1);
    text.text = String::fromUTF8(buf.mutableSpan().data());
}

void Pasteboard::read(PasteboardWebContentReader& reader, WebContentReadingPolicy, std::optional<size_t>)
{
    notImplemented();
}

bool Pasteboard::hasData()
{
    return wkcPasteboardIsFormatAvailablePeer(WKC_CLIPBOARD_FORMAT_HTML)
        || wkcPasteboardIsFormatAvailablePeer(WKC_CLIPBOARD_FORMAT_TEXT);
}

Vector<String> Pasteboard::typesSafeForBindings(const String&)
{
    notImplemented();
    return { };
}

Vector<String> Pasteboard::typesForLegacyUnsafeBindings()
{
    notImplemented();
    return { };
}

String Pasteboard::readOrigin()
{
    notImplemented();
    return String();
}

String Pasteboard::readString(const String&)
{
    notImplemented();
    return String();
}

String Pasteboard::readStringInCustomData(const String&)
{
    notImplemented();
    return String();
}

void Pasteboard::writeString(const String&, const String&)
{
    notImplemented();
}

void Pasteboard::clear(const String&)
{
    wkcPasteboardClearPeer();
}

} // namespace WebCore
