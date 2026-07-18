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

#ifndef _WKC_HELPERS_WKC_ATOMICSTRING_H_
#define _WKC_HELPERS_WKC_ATOMICSTRING_H_

namespace WKC {
class String;
class AtomString;
class AtomStringPrivate;
} // namespace
/*
namespace WKC {
    extern const AtomString nullAtom;
} // namespace
*/
namespace WKC {

class WKC_API AtomString {
public:
    AtomString();
    AtomString(AtomStringPrivate* priv);
    ~AtomString();

    AtomString(const AtomString&);
    AtomString& operator=(const AtomString&);

    const String& string() const;

    AtomStringPrivate* priv() const { return m_private; }

private:
    AtomStringPrivate* m_private;
    bool m_privateowner;
    //WKC_DEFINE_GLOBAL_CLASS_OBJ_ENTRY(AtomString*, m_nullAtom);

};

} // namespace

#endif //_WKC_HELPERS_WKC_ATOMICSTRING_H_
