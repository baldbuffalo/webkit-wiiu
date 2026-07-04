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

#ifndef _WKC_HELPERS_PRIVATE_QUALIFIEDNAME_H_
#define _WKC_HELPERS_PRIVATE_QUALIFIEDNAME_H_

#include <wtf/text/WTFString.h>

namespace WebCore {
class QualifiedName;
} // namespace

namespace WTF {
class AtomString;
}

namespace WKC {
class QualifiedName;
class AtomString;
} // namespace

namespace WKC {

class QualifiedNamePrivate {
    friend class QualifiedName;
public:
    QualifiedNamePrivate(const WKC::AtomString&, const String&, const WKC::AtomString&);
    ~QualifiedNamePrivate();

    WebCore::QualifiedName* webcore() const { return m_webcore; }

private:
    QualifiedNamePrivate(const WTF::AtomString&, const String&, const WTF::AtomString&);

private:
    WebCore::QualifiedName* m_webcore;
    WTF::String m_webcore_local_name;
    WTF::AtomString* m_webcore_prefix;
    WTF::AtomString* m_webcore_namespace;
};
} // namespace

#endif //_WKC_HELPERS_PRIVATE_QUALIFIEDNAME_H_
