/*
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
 * Modernized for current WebKit (WTF::URL) — webkit-wiiu.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include <wtf/URL.h>
#include <wtf/text/StringView.h>
#include <pal/text/TextEncoding.h>
#include "helpers/WKCURL.h"
#include "helpers/WKCString.h"

#define PARENT() ((WTF::URL *)m_parent)

namespace WKC {

// WebCore::URL is now WTF::URL, and the URL(ParsedURLString, str) constructor
// was removed. Parsing an absolute URL string is now URL(URL(), string).

URL::URL(URLPrivate* parent)
    : m_parent((URLPrivate *)new WTF::URL(*((WTF::URL *)parent)))
{
}

URL::URL()
    : m_parent((URLPrivate *)new WTF::URL())
{
}

URL::URL(const URL& base, const char* str)
    : m_parent((URLPrivate *)new WTF::URL(*((WTF::URL *)base.parent()), WTF::String::fromLatin1(str)))
{
}

URL::URL(const URL& url)
    : m_parent((URLPrivate *)new WTF::URL(*((WTF::URL *)url.parent())))
{
}

URL::URL(WKCURLParsedEnum, const char* url)
    : m_parent((URLPrivate *)new WTF::URL(WTF::URL(), WTF::String::fromLatin1(url)))
{
}

URL::~URL()
{
    delete (WTF::URL *)m_parent;
}

URL&
URL::operator=(const URL& orig)
{
    if (this != &orig) {
        delete (WTF::URL *)m_parent;
        m_parent = (URLPrivate *)new WTF::URL(*((WTF::URL *)orig.parent()));
    }
    return *this;
}


URL::operator String() const
{
    return string();
}

const String
URL::string() const
{
    return PARENT()->string();
}

const String
WKC::URL::protocol() const
{
    return PARENT()->protocol().toString();
}

const String
WKC::URL::host() const
{
    return PARENT()->host().toString();
}

unsigned short
WKC::URL::port() const
{
    return PARENT()->port().value_or(0);
}

const String
WKC::URL::path() const
{
    return PARENT()->path().toString();
}

const String
WKC::URL::lastPathComponent() const
{
    return PARENT()->lastPathComponent().toString();
}

String
decodeURLEscapeSequences(const String& str)
{
    return PAL::decodeURLEscapeSequences(WTF::String(str));
}

String
encodeWithURLEscapeSequences(const String& str)
{
    return WTF::encodeWithURLEscapeSequences(WTF::String(str));
}

bool
protocolIs(const String& url, const char* protocol)
{
    // Extra parens around the first argument avoid the most-vexing-parse
    // (otherwise 'u' is read as a function declaration).
    WTF::URL u((WTF::URL()), WTF::String(url));
    return equalIgnoringASCIICase(u.protocol(), WTF::String::fromLatin1(protocol));
}

} // namespace WKC

// Interop between WTF::URL and the WKC::URL wrapper. These are declared on
// WTF::URL by the CI header-injection step (see .github/workflows/build.yml).
namespace WTF {
URL::URL(const WKC::URL& url)
{
    if (WTF::URL* parent = (WTF::URL *)url.parent())
        *this = *parent;
}

URL::operator ::WKC::URL() const
{
    return ::WKC::URL((::WKC::URLPrivate *)this);
}
} // namespace WTF
