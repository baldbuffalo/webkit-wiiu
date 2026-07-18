/*
 * Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#include "helpers/WKCHTMLElement.h"
#include "helpers/privates/WKCHTMLElementPrivate.h"
#include "helpers/WKCString.h"

#include "HTMLElement.h"
#include <wtf/Expected.h>
#include "ExceptionOr.h"

namespace WKC {

HTMLElementPrivate::HTMLElementPrivate(WebCore::HTMLElement* parent)
    : ElementPrivate(parent)
    , m_webcore(parent)
    , m_wkc(*this)
{
}

HTMLElementPrivate::~HTMLElementPrivate()
{
}

void
HTMLElementPrivate::setInnerText(const String& text, int& ec)
{
    // setInnerText now takes String&& and returns ExceptionOr<void> (no ec out-param).
    ec = webcore()->setInnerText(WTF::String(text)).hasException() ? 1 : 0;
}

HTMLElement::HTMLElement(HTMLElementPrivate& parent)
    : Element(parent)
{
}

HTMLElement::~HTMLElement()
{
}

void
HTMLElement::setInnerText(const String& text, int& ec)
{
    ((HTMLElementPrivate&)priv()).setInnerText(text, ec);
}

} // namespace
