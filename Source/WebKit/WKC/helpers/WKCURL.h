/*
 * Copyright (c) 2011, 2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_WKC_URL_H_
#define _WKC_HELPERS_WKC_URL_H_

#include <wkc/wkcbase.h>

namespace WKC {
class String;

enum WKCURLParsedEnum { WKCURLParsed };

class URLPrivate;

class WKC_API URL {
public:
    URL();
    URL(const URL&, const char*);
    URL(WKCURLParsedEnum, const char*);
    ~URL();

    URL& operator=(const URL&);
    operator String() const;

    const String string() const;
    const String protocol() const;
    const String host() const;
    unsigned short port() const;
    const String path() const;
    const String lastPathComponent() const;

    URL(URLPrivate*);
    URL(const URL&);

    URLPrivate* parent() const { return m_parent; }

private:
    URLPrivate* m_parent;
};

WKC_API String decodeURLEscapeSequences(const String&);
WKC_API String encodeWithURLEscapeSequences(const String&);
WKC_API bool protocolIs(const String& url, const char* protocol);

} // namespace

#endif // _WKC_HELPERS_WKC_URL_H_
