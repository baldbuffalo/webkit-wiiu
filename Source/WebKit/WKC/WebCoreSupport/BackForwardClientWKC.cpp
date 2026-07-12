/*
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
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

#include "HistoryItem.h"

#include <wtf/Vector.h>

#include "BackForwardClientWKC.h"
#include "WKCWebViewPrivate.h"

#include "helpers/BackForwardClientIf.h"

#include "helpers/privates/WKCHistoryItemPrivate.h"

namespace WKC {

BackForwardClientWKC::BackForwardClientWKC(WKCWebViewPrivate* view)
     : m_view(view),
       m_appClient(0)
{
}

BackForwardClientWKC::~BackForwardClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteBackForwardClient(m_appClient);
        m_appClient = 0;
    }
}

Ref<BackForwardClientWKC>
BackForwardClientWKC::create(WKCWebViewPrivate* view)
{
    auto self = adoptRef(*new BackForwardClientWKC(view));
    self->construct();
    return self;
}

bool
BackForwardClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createBackForwardClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
BackForwardClientWKC::addItem(Ref<WebCore::HistoryItem>&& item)
{
    HistoryItemPrivate wobj(item.ptr());
    m_appClient->addItem(&wobj.wkc());
}

void
BackForwardClientWKC::setChildItem(WebCore::BackForwardFrameItemIdentifier, Ref<WebCore::HistoryItem>&&)
{
    // The WKC application history interface models main-frame items only, so
    // per-subframe items carry no destination in this layer.
}

void
BackForwardClientWKC::goToItem(WebCore::HistoryItem& item)
{
    HistoryItemPrivate wobj(&item);
    m_appClient->goToItem(&wobj.wkc());
}

Vector<Ref<WebCore::HistoryItem>>
BackForwardClientWKC::allItems(WebCore::FrameIdentifier)
{
    Vector<Ref<WebCore::HistoryItem>> items;
    int back = static_cast<int>(m_appClient->backListCount());
    int forward = static_cast<int>(m_appClient->forwardListCount());
    for (int i = -back; i <= forward; ++i) {
        HistoryItem* item = m_appClient->itemAtIndex(i);
        if (item && item->priv().webcore())
            items.append(*item->priv().webcore());
    }
    return items;
}

RefPtr<WebCore::HistoryItem>
BackForwardClientWKC::itemAtIndex(int index, WebCore::FrameIdentifier)
{
    HistoryItem* item = m_appClient->itemAtIndex(index);
    if (item)
        return item->priv().webcore();
    return nullptr;
}

unsigned
BackForwardClientWKC::backListCount() const
{
    return static_cast<unsigned>(m_appClient->backListCount());
}

unsigned
BackForwardClientWKC::forwardListCount() const
{
    return static_cast<unsigned>(m_appClient->forwardListCount());
}

bool
BackForwardClientWKC::containsItem(const WebCore::HistoryItem& target) const
{
    int back = static_cast<int>(m_appClient->backListCount());
    int forward = static_cast<int>(m_appClient->forwardListCount());
    for (int i = -back; i <= forward; ++i) {
        HistoryItem* item = m_appClient->itemAtIndex(i);
        if (item && item->priv().webcore() == &target)
            return true;
    }
    return false;
}

void
BackForwardClientWKC::close()
{
    m_appClient->close();
}


} // namespace

