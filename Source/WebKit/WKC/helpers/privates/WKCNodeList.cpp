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

// Must precede everything: NodeList.h's chain (JSExecState.h,
// CachedResourceRequestInitiatorTypes.h) calls WebCore::threadGlobalDataSingleton()
// unqualified. A bare "ThreadGlobalData.h" resolves to PAL's copy here, so force
// WebCore's platform version by explicit path to declare it first; otherwise the
// chain fails to compile and NodeList is never declared in WebCore.
#include "../../../../WebCore/platform/ThreadGlobalData.h"
#include "Node.h"

#include "helpers/WKCNodeList.h"
#include "helpers/privates/WKCNodeListPrivate.h"

#include "NodeList.h"

#include "helpers/privates/WKCNodePrivate.h"

namespace WKC {

// Private Implementation

NodeListPrivate::NodeListPrivate(RefPtr<WebCore::NodeList>&& parent)
     : m_webcore(parent.get())
     , m_wkc(*this)
     , m_refptr(WTFMove(parent))
{
}

NodeListPrivate::~NodeListPrivate() = default;

unsigned
NodeListPrivate::length() const
{
    return m_webcore->length();
}

Node*
NodeListPrivate::item(unsigned index)
{
    WebCore::Node* node = m_webcore->item(index);
    if (!node)
        return nullptr;
    if (!m_node || m_node->webcore() != node)
        m_node = std::unique_ptr<NodePrivate>(NodePrivate::create(node));
    return &m_node->wkc();
}

// Implementation

NodeList::NodeList(NodeListPrivate& parent)
     : m_private(parent)
{
}

NodeList::~NodeList()
{
}

unsigned
NodeList::length() const
{
    return m_private.length();
}

Node*
NodeList::item(unsigned index) const
{
    return m_private.item(index);
}

} // namespace
