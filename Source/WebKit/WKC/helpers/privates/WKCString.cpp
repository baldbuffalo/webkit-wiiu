/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2011 Torch Mobile, Inc.
 * Copyright (c) 2011-2012 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "CString.h"
#include <wtf/text/WTFString.h>
#include "StringImpl.h"

#include "helpers/WKCString.h"

#include <cstring>
#include <span>
#include <wtf/Vector.h>
#include <wtf/text/MakeString.h>
#include <wtf/text/StringView.h>

namespace WKC {


CString::CString(const char* data, int len)
    : m_parent(new WTF::CString(std::span<const char> { data, static_cast<size_t>(len) }))
{
}

CString::~CString()
{
    delete (WTF::CString *)m_parent;
}

CString&
CString::operator=(const CString& other)
{
    if (this!=&other) {
        m_parent = new WTF::CString(std::span<const char> { other.data(), static_cast<size_t>(other.length()) });
    }
    return *this;
}

#define CPARENT() ((WTF::CString *)m_parent)

const char*
CString::data() const
{
    return CPARENT()->data();
}


int
CString::length() const
{
    return CPARENT()->length();
}

// --- helpers -------------------------------------------------------------

// Modern WTF::String is immutable and stores either Latin-1 (8-bit) or UTF-16
// (16-bit) internally. The 2011-era WKC helpers below rebuild a fresh String for
// every "mutating" operation (append/replace/remove/insert/truncate), which is
// what the modern API expects.
static inline WTF::String stringFrom16(const unsigned short* str, unsigned len)
{
    return WTF::String(WTF::StringImpl::create(std::span<const char16_t> { reinterpret_cast<const char16_t*>(str), len }));
}

static inline unsigned length16(const unsigned short* str)
{
    unsigned len = 0;
    for (const unsigned short* p = str; p && *p; ++p)
        ++len;
    return len;
}

class StringPrivate {
public:
    StringPrivate();
    ~StringPrivate();

    inline void setImpl(WTF::RefPtr<WTF::StringImpl> i) { if (i) m_impl = i; }
    inline WTF::StringImpl* impl() const { return m_impl.get(); }

    const unsigned short* characters();
    const unsigned short* charactersWithNullTermination();
    CString& latin1();
    CString& utf8();

    inline void setDirection(TextDirection dir) { m_direction = dir; }
    inline TextDirection direction() const { return m_direction; }

private:
    WTF::RefPtr<WTF::StringImpl> m_impl;
    CString* m_latin1;
    CString* m_utf8;

    // Cached UTF-16 upconversions backing characters() /
    // charactersWithNullTermination(). StringImpl no longer exposes a stable
    // characters() pointer (it may be 8-bit), so keep our own buffer alive.
    WTF::Vector<char16_t> m_charactersBuffer;
    WTF::Vector<char16_t> m_charactersNullTerminatedBuffer;

    TextDirection m_direction;
};

StringPrivate::StringPrivate()
    : m_latin1(0)
    , m_utf8(0)
    , m_direction(LTR)
{
}

StringPrivate::~StringPrivate()
{
    delete m_latin1;
    delete m_utf8;
}

const unsigned short*
StringPrivate::characters()
{
    if (!m_impl)
        return 0;
    WTF::String s(m_impl.get());
    unsigned len = s.length();
    m_charactersBuffer.resize(len);
    for (unsigned i = 0; i < len; ++i)
        m_charactersBuffer[i] = s[i];
    return reinterpret_cast<const unsigned short*>(m_charactersBuffer.span().data());
}

const unsigned short*
StringPrivate::charactersWithNullTermination()
{
    if (!m_impl)
        return 0;
    WTF::String s(m_impl.get());
    unsigned len = s.length();
    m_charactersNullTerminatedBuffer.resize(len + 1);
    for (unsigned i = 0; i < len; ++i)
        m_charactersNullTerminatedBuffer[i] = s[i];
    m_charactersNullTerminatedBuffer[len] = 0;
    return reinterpret_cast<const unsigned short*>(m_charactersNullTerminatedBuffer.span().data());
}

CString&
StringPrivate::latin1()
{
    delete m_latin1;

    WTF::String a(m_impl.get());
    WTF::CString cs = a.latin1();
    if (!cs.isNull()) {
        m_latin1 = new CString(cs.data(), cs.length());
    } else {
        m_latin1 = new CString(0,0);
    }
    return *m_latin1;
}

CString&
StringPrivate::utf8()
{
    delete m_utf8;

    WTF::String a(m_impl.get());
    WTF::CString cs = a.utf8();
    if (!cs.isNull()) {
        m_utf8 = new CString(cs.data(), cs.length());
    } else {
        m_utf8 = new CString(0,0);
    }
    return *m_utf8;
}


#define IMPL() (m_private ? (m_private->impl()) : 0)

String::String()
    : m_private()
{
}

String::String(const char* str)
{
    m_private = new StringPrivate();
    m_private->setImpl(WTF::String::fromLatin1(str).impl());
}

String::String(const char* str, unsigned int len)
{
    m_private = new StringPrivate();
    m_private->setImpl(WTF::StringImpl::create(std::span<const char> { str, len }));
}

String::String(const unsigned short* str)
{
    m_private = new StringPrivate();
    m_private->setImpl(stringFrom16(str, length16(str)).impl());
}

String::String(const unsigned short* str, unsigned int len)
{
    m_private = new StringPrivate();
    m_private->setImpl(stringFrom16(str, len).impl());
}

String::String(const String& str)
{
    m_private = new StringPrivate();

    WTF::StringImpl* impl = str.impl()->impl();
    if (impl) {
        m_private->setImpl(impl);
    }
}

String::String(StringPrivate* parent)
{
    m_private = parent;
}

String::~String()
{
    delete m_private;
}

String&
String::operator=(const String& orig)
{
    if (this!=&orig) {
        if (!m_private)
            m_private = new StringPrivate();
        m_private->setImpl(orig.m_private->impl());
    }
    return *this;
}

bool
String::operator==(const char* b) const
{
    // WTF::String lost operator==(const char*); compare against a Latin-1 String.
    const WTF::String a(IMPL());
    return a == WTF::String::fromLatin1(b);
}

bool
String::operator!=(const char* b) const
{
    const WTF::String a(IMPL());
    return a != WTF::String::fromLatin1(b);
}


void
String::append(const String& str)
{
    WTF::String a(IMPL());
    WTF::String b(str);
    m_private->setImpl(WTF::makeString(a, b).impl());
}


void
String::append(const char* str)
{
    WTF::String a(IMPL());
    m_private->setImpl(WTF::makeString(a, WTF::String::fromLatin1(str)).impl());
}

void
String::append(const unsigned short* str)
{
    WTF::String a(IMPL());
    m_private->setImpl(WTF::makeString(a, stringFrom16(str, length16(str))).impl());
}

void
String::append(const unsigned short* str, unsigned int len)
{
    WTF::String a(IMPL());
    m_private->setImpl(WTF::makeString(a, stringFrom16(str, len)).impl());
}

size_t
String::find(const String& str)
{
    const WTF::String a(IMPL());
    WTF::String b(str);
    return a.find(WTF::StringView(b));
}

size_t
String::find(unsigned short ch, int len)
{
    const WTF::String a(IMPL());
    return a.find(static_cast<char16_t>(ch), static_cast<unsigned>(len));
}

size_t
String::reverseFind(unsigned short ch) const
{
    const WTF::String a(IMPL());
    size_t ret = a.reverseFind(static_cast<char16_t>(ch));
    return ret == WTF::notFound ? WKC::notFound : ret;
}

String&
String::replace(const unsigned short* a, const unsigned short* b)
{
    WTF::String s(IMPL());
    WTF::String target = stringFrom16(a, length16(a));
    WTF::String replacement = stringFrom16(b, length16(b));
    s = WTF::makeStringByReplacingAll(s, WTF::StringView(target), WTF::StringView(replacement));
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(const unsigned short* a, const String& b)
{
    WTF::String s(IMPL());
    WTF::String target = stringFrom16(a, length16(a));
    WTF::String replacement(b);
    s = WTF::makeStringByReplacingAll(s, WTF::StringView(target), WTF::StringView(replacement));
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(const String& a, const String& b)
{
    WTF::String s(IMPL());
    WTF::String target(a);
    WTF::String replacement(b);
    s = WTF::makeStringByReplacingAll(s, WTF::StringView(target), WTF::StringView(replacement));
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(unsigned index, unsigned len, const String& b)
{
    WTF::String s(IMPL());
    WTF::String replacement(b);
    s = WTF::makeString(s.substring(0, index), replacement, s.substring(index + len));
    m_private->setImpl(s.impl());
    return *this;
}

void
String::truncate(unsigned int len)
{
    WTF::String a(IMPL());
    m_private->setImpl(a.left(len).impl());
}

WKC::String
String::substring(unsigned int pos, unsigned int len) const
{
    WTF::String a(IMPL());
    return a.substring(pos, len);
}

WKC::String
String::lower() const
{
    WTF::String a(IMPL());
    return a.convertToLowercaseWithoutLocale();
}

WKC::String
String::upper() const
{
    WTF::String a(IMPL());
    return a.convertToUppercaseWithoutLocale();
}

WKC::String
String::fromUTF8(const char* utf8)
{
    return WTF::String::fromUTF8(std::span<const char8_t> { reinterpret_cast<const char8_t*>(utf8), utf8 ? std::strlen(utf8) : 0 });
}

WKC::String
String::fromUTF8(const char* utf8, size_t len)
{
    return WTF::String::fromUTF8(std::span<const char8_t> { reinterpret_cast<const char8_t*>(utf8), len });
}

void
String::remove(unsigned int pos, unsigned int len)
{
    WTF::String a(IMPL());
    WTF::String result = WTF::makeString(a.substring(0, pos), a.substring(pos + len));
    m_private->setImpl(result.impl());
}

void
String::insert(const unsigned short* str, unsigned int pos, unsigned int len)
{
    WTF::String a(IMPL());
    WTF::String result = WTF::makeString(a.substring(0, pos), stringFrom16(str, len), a.substring(pos));
    m_private->setImpl(result.impl());
}

WKC::String
String::format(const char * format, ...)
{
    // copied from String.cpp

    va_list args;
    va_start(args, format);

    WTF::Vector<char, 256> buffer;

    // Do the format once to get the length.
#if COMPILER(MSVC)
    int result = _vscprintf(format, args);
#else
    char ch;
    int result = vsnprintf(&ch, 1, format, args);
    va_end(args);
    va_start(args, format);
#endif
    if (result==0) {
        return String("");
    } else if (result<0) {
        return String();
    }

    unsigned len = result;
    buffer.grow(len + 1);

    vsnprintf(buffer.data(), buffer.size(), format, args);

    va_end(args);

    return String(buffer.data(), len);
}



const unsigned short*
String::characters() const
{
    if (!m_private || !m_private->impl())
        return 0;
    return m_private->characters();
}

unsigned int
String::length() const
{
    if (!m_private || !m_private->impl())
        return 0;
    return IMPL()->length();
}

CString&
String::utf8() const
{
    return m_private->utf8();
}

CString&
String::latin1() const
{
    return m_private->latin1();
}

const unsigned short*
String::charactersWithNullTermination() const
{
    return m_private->charactersWithNullTermination();
}


bool
String::isNull() const
{
    const WTF::String a(IMPL());
    return a.isNull();
}

bool
String::isEmpty() const
{
    const WTF::String a(IMPL());
    return a.isEmpty();
}

TextDirection
String::direction() const
{
    return m_private->direction();
}

void
String::setDirection(TextDirection dir)
{
    m_private->setDirection(dir);
}


} // namespace

namespace WTF {
String::String(const WKC::String& str)
    : m_impl(0)
{
    if (str.impl()) {
        m_impl = str.impl()->impl();
    }
}

String::operator ::WKC::String() const
{
    ::WKC::StringPrivate* obj = new ::WKC::StringPrivate();

    obj->setImpl(impl());

    return ::WKC::String(obj);
}

} // namespace
