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

#include "helpers/WKCDocument.h"
#include "helpers/privates/WKCDocumentPrivate.h"

#include "Document.h"
// Document::frame() returns LocalFrame* now; include the concrete types so the
// LocalFrame* -> Frame* upcast is visible.
#include "Frame.h"
#include "LocalFrame.h"
#include "DocumentResourceLoader.h"
#include "FrameDestructionObserverInlines.h"
#include "helpers/privates/WKCCachedResourceLoaderPrivate.h"
#include "helpers/privates/WKCHTMLCollectionPrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCRenderViewPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/WKCURL.h"

namespace WKC {

DocumentPrivate::DocumentPrivate(WebCore::Document* parent)
    : NodePrivate(parent)
    , m_webcore(parent)
    , m_wkc(*this)
    , m_cachedResourceLoader(0)
    , m_focusedNode(0)
    , m_renderView(0)
    , m_frame(0)
    , m_firstChild(0)
    , m_collection(0)
{
}

DocumentPrivate::~DocumentPrivate()
{
    delete m_firstChild;
    delete m_frame;
    delete m_renderView;
    delete m_focusedNode;
    delete m_cachedResourceLoader;
    delete m_collection;
}

bool
DocumentPrivate::isImageDocument() const
{
    return m_webcore->isImageDocument();
}

bool
DocumentPrivate::isSVGDocument() const
{
    return m_webcore->isSVGDocument();
}

void
DocumentPrivate::updateLayoutIgnorePendingStylesheets()
{
    m_webcore->updateLayoutIgnorePendingStylesheets();
}

URL
DocumentPrivate::completeURL(const String& url) const
{
    // 2026: Document::completeURL was renamed to encodingParseURL (ScriptExecutionContext).
    return m_webcore->encodingParseURL(url);
}

Node*
DocumentPrivate::focusedNode()
{
    // 2026: Document::focusedNode() renamed to focusedElement() (returns Element*).
    WebCore::Node* node = m_webcore->focusedElement();
    if (!node)
        return 0;
    if (!m_focusedNode || m_focusedNode->webcore()!=node) {
        delete m_focusedNode;
        m_focusedNode = NodePrivate::create(node);
    }
    return &m_focusedNode->wkc();
}

RenderView*
DocumentPrivate::renderView() 
{
    WebCore::RenderView* view = m_webcore->renderView();
    if (!view)
        return 0;
    if (!m_renderView || m_renderView->webcore()!=view) {
        delete m_renderView;
        m_renderView = new RenderViewPrivate(view);
    }
    return &m_renderView->wkc();
}

CachedResourceLoader*
DocumentPrivate::cachedResourceLoader()
{
    // 2026: cachedResourceLoader() returns a reference now.
    WebCore::CachedResourceLoader* loader = &m_webcore->cachedResourceLoader();
    if (!loader)
        return 0;
    if (!m_cachedResourceLoader || m_cachedResourceLoader->webcore()!=loader) {
        delete m_cachedResourceLoader;
        m_cachedResourceLoader = new CachedResourceLoaderPrivate(loader);
    }
    return &m_cachedResourceLoader->wkc();
}

Frame*
DocumentPrivate::frame()
{
    WebCore::Frame* frame = m_webcore->frame();
    if (!frame)
        return 0;
    if (!m_frame || m_frame->webcore()!=frame) {
        delete m_frame;
        m_frame = new FramePrivate(frame);
    }
    return &m_frame->wkc();
}

Node*
DocumentPrivate::firstChild()
{
    WebCore::Node* node = m_webcore->firstChild();
    if (!node)
        return 0;
    if (!m_firstChild || m_firstChild->webcore()!=node) {
        delete m_firstChild;
        m_firstChild = NodePrivate::create(node);
    }
    return &m_firstChild->wkc();
}

HTMLCollection*
DocumentPrivate::forms()
{
    RefPtr<WebCore::HTMLCollection> coll = m_webcore->forms();
    if (!coll)
        return 0;
    delete m_collection;
    m_collection = new HTMLCollectionPrivate(coll);
    return &m_collection->wkc();
}

bool
DocumentPrivate::loadEventFinished() const
{
    return m_webcore->loadEventFinished();
}

Document::Document(DocumentPrivate& parent)
    : Node(parent)
    , m_private(parent)
{
}

Document::~Document()
{
}

Node*
Document::focusedNode() const
{
    return m_private.focusedNode();
}


Frame*
Document::frame() const
{
    return m_private.frame();
}

Node*
Document::firstChild() const
{
    return m_private.firstChild();
}

CachedResourceLoader*
Document::cachedResourceLoader() const
{
    return m_private.cachedResourceLoader();
}

bool
Document::isImageDocument() const
{
    return m_private.isImageDocument();
}

bool
Document::isSVGDocument() const
{
    return m_private.isSVGDocument();
}

RenderView*
Document::renderView() const
{
    return m_private.renderView();
}

void
Document::updateLayoutIgnorePendingStylesheets()
{
    m_private.updateLayoutIgnorePendingStylesheets();
}

URL
Document::completeURL(const String& url) const
{
    return m_private.completeURL(url);
}

HTMLCollection*
Document::forms() const
{
    return m_private.forms();
}

bool
Document::loadEventFinished() const
{
    return m_private.loadEventFinished();
}

} // namespace
