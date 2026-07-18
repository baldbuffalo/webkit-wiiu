/*
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCAtomString.h"
#include "helpers/privates/WKCAtomStringPrivate.h"

#include "helpers/WKCString.h"

#include <wtf/text/AtomString.h>
/*
namespace WKC {
    //const AtomString nullAtom;
    //DEFINE_STATIC_LOCAL(AtomString, nullAtom, 0);
    //WKC_DEFINE_GLOBAL_PTR(const WKC::AtomString, nullAtom, 0);
    WKC_DEFINE_GLOBAL_CLASS_OBJ(AtomString*, AtomString, m_nullAtom, 0);
} // namespace
*/
namespace WKC {

AtomStringPrivate::AtomStringPrivate()
    : m_webcoreowner(1)
{
    m_wkc = 0;
    m_webcore = new WTF::AtomString();
}

AtomStringPrivate::AtomStringPrivate(WTF::AtomString* value)
    : m_webcore(value)
    , m_webcoreowner(0)
{
    m_wkc = new AtomString(this);
}

AtomStringPrivate::~AtomStringPrivate()
{
    delete m_wkc;
    if (m_webcoreowner)
        delete m_webcore;
}

AtomStringPrivate::AtomStringPrivate(const AtomStringPrivate& other)
{
    if (this!=&other)
        ::memcpy(this, &other, sizeof(AtomStringPrivate));
}

AtomStringPrivate&
AtomStringPrivate::operator =(const AtomStringPrivate& other)
{
    if (this!=&other) {
        ::memcpy(this, &other, sizeof(AtomStringPrivate));
    }

    return *this;
}

String&
AtomStringPrivate::string()
{
    m_wkc_string = m_webcore->string();
    return m_wkc_string;
}

AtomString::AtomString()
    : m_privateowner(true)
{
    m_private = new AtomStringPrivate();
}

AtomString::AtomString(AtomStringPrivate* priv)
    : m_privateowner(false)
{
    m_private = priv;
}

AtomString::AtomString(const AtomString& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_privateowner = false;
    }
}

AtomString&
AtomString::operator =(const AtomString& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_privateowner = false;
    }
    return *this;
}

AtomString::~AtomString()
{
    if (m_privateowner)
        delete m_private;
}

const String&
AtomString::string() const
{
    return m_private->string();
}

} // namespace