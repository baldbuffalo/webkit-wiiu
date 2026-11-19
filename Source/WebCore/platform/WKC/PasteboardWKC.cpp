#include "config.h"
#include "Pasteboard.h"

#include "DocumentFragment.h"
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
    return makeUnique<Pasteboard>(std::move(context));
}

Pasteboard::Pasteboard(std::unique_ptr<PasteboardContext>&& context)
    : m_context(std::move(context))
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
    wkcPasteboardWritePlainTextPeer(utf8.legacyCStringPointer(), static_cast<int>(utf8.length()));
}

void Pasteboard::write(const PasteboardURL& pasteboardURL)
{
    clear();
    auto urlStr = pasteboardURL.url.string().utf8();
    auto titleStr = pasteboardURL.title.utf8();
    wkcPasteboardWriteURIPeer(urlStr.legacyCStringPointer(), static_cast<int>(urlStr.length()),
                               titleStr.legacyCStringPointer(), static_cast<int>(titleStr.length()));
}

void Pasteboard::write(const PasteboardWebContent& content)
{
    notImplemented();
}

void Pasteboard::write(PasteboardImage&&)
{
    // The WKC peer clipboard currently exposes text and URI formats only.
    // Modern PasteboardImage no longer carries the legacy Image* payload.
    notImplemented();
}

void Pasteboard::writeCustomData(const Vector<PasteboardCustomData>&, PasteboardWriteType)
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
