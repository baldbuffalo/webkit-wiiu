/*
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *  Copyright (C) 2007 Alp Toker <alp@atoker.com>
 *  Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "Pasteboard.h"

#include "DocumentFragment.h"
#include "Frame.h"
#include "Image.h"
#include "ImageWKC.h"
#include "NotImplemented.h"
#include "PasteboardCustomData.h"
#include "RenderImage.h"
#include "SerializedAttachmentData.h"
#include "markup.h"
#include <wtf/URL.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <wkc/wkcpeer.h>
#include <wkc/wkcgpeer.h>

namespace WebCore {

std::unique_ptr<Pasteboard> Pasteboard::createForCopyAndPaste(std::unique_ptr<PasteboardContext> context)
{
    return makeUnique<Pasteboard>(WTFMove(context));
}

Pasteboard::Pasteboard(std::unique_ptr<PasteboardContext> context)
    : m_context(WTFMove(context))
{
}

Pasteboard::~Pasteboard()
{
}

void Pasteboard::clear()
{
    wkcPasteboardClearPeer();
}

void Pasteboard::writePlainText(const String& text, SmartReplaceOption)
{
    clear();
    wkcPasteboardWritePlainTextPeer(text.utf8().data(), (int)text.utf8().length());
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
    clear();
    if (!content.markup.isEmpty()) {
        auto html = content.markup.utf8();
        auto url = content.urlString.utf8();
        auto plain = content.text.utf8();
        wkcPasteboardWriteHTMLPeer(html.data(), (int)html.length(),
                                   url.data(), (int)url.length(),
                                   plain.data(), (int)plain.length(),
                                   content.canSmartCopyOrDelete);
    }
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
    wkcPasteboardWriteImagePeer(type, bitmap->bitmap(), bitmap->rowbytes(), 0, 0, &size);
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
    len = wkcPasteboardReadPlainTextPeer(buf.data(), len + 1);
    text.text = String::fromUTF8(buf.data(), len);
}

void Pasteboard::read(PasteboardWebContentReader& reader, WebContentReadingPolicy, std::optional<size_t>)
{
    if (wkcPasteboardIsFormatAvailablePeer(WKC_CLIPBOARD_FORMAT_HTML)) {
        int len = wkcPasteboardReadHTMLPeer(nullptr, 0);
        if (len > 0) {
            Vector<char> buf(len + 1);
            len = wkcPasteboardReadHTMLPeer(buf.data(), len + 1);
            String html = String::fromUTF8(buf.data(), len);

            int ulen = wkcPasteboardReadHTMLURIPeer(nullptr, 0);
            String url;
            if (ulen > 0) {
                Vector<char> ubuf(ulen + 1);
                ulen = wkcPasteboardReadHTMLURIPeer(ubuf.data(), ulen + 1);
                url = String::fromUTF8(ubuf.data(), ulen);
            }
            if (!html.isEmpty())
                reader.readHTML(html, url, { });
        }
    }

    if (wkcPasteboardIsFormatAvailablePeer(WKC_CLIPBOARD_FORMAT_TEXT)) {
        int len = wkcPasteboardReadPlainTextPeer(nullptr, 0);
        if (len > 0) {
            Vector<char> buf(len + 1);
            len = wkcPasteboardReadPlainTextPeer(buf.data(), len + 1);
            reader.readPlainText(String::fromUTF8(buf.data(), len));
        }
    }
}

void Pasteboard::read(PasteboardCustomData&)
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
