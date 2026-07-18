/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DragClientWKC.h"
#include "WKCWebViewPrivate.h"
#include "helpers/DragClientIf.h"

#include "helpers/WKCURL.h"
#include "helpers/WKCString.h"

#include "DragData.h"
#include "DataTransfer.h"
#include "DragItem.h"
#include "Frame.h"
#include <wtf/text/WTFString.h>
#include <wtf/URL.h>

#include "helpers/privates/WKCDataTransferPrivate.h"
#include "helpers/privates/WKCDragDataPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"

// implementations

namespace WKC {

DragClientWKC::DragClientWKC(WKCWebViewPrivate* view)
     : m_view(view),
       m_appClient(0)
{
}

DragClientWKC::~DragClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteDragClient(m_appClient);
        m_appClient = 0;
    }
}

DragClientWKC*
DragClientWKC::create(WKCWebViewPrivate* view)
{
    DragClientWKC* self = 0;
    self = new DragClientWKC(view);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
DragClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createDragClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
DragClientWKC::willPerformDragDestinationAction(WebCore::DragDestinationAction action, const WebCore::DragData& data)
{
    DragDataPrivate d(const_cast<WebCore::DragData*>(&data));
    m_appClient->willPerformDragDestinationAction(static_cast<WKC::DragDestinationAction>(static_cast<unsigned>(action)), &d.wkc());
}

void
DragClientWKC::willPerformDragSourceAction(WebCore::DragSourceAction action, const WebCore::IntPoint& pos, WebCore::DataTransfer& dataTransfer)
{
    DataTransferPrivate dt(&dataTransfer);
    m_appClient->willPerformDragSourceAction(static_cast<WKC::DragSourceAction>(static_cast<unsigned>(action)), pos, &dt.wkc());
}

OptionSet<WebCore::DragSourceAction>
DragClientWKC::dragSourceActionMaskForPoint(const WebCore::IntPoint& rootViewPoint)
{
    unsigned mask = static_cast<unsigned>(m_appClient->dragSourceActionMaskForPoint(rootViewPoint));
    OptionSet<WebCore::DragSourceAction> result;
    if (mask & WKC::DragSourceActionDHTML)
        result.add(WebCore::DragSourceAction::DHTML);
    if (mask & WKC::DragSourceActionImage)
        result.add(WebCore::DragSourceAction::Image);
    if (mask & WKC::DragSourceActionLink)
        result.add(WebCore::DragSourceAction::Link);
    if (mask & WKC::DragSourceActionSelection)
        result.add(WebCore::DragSourceAction::Selection);
    return result;
}

void
DragClientWKC::startDrag(WebCore::DragItem item, WebCore::DataTransfer& dataTransfer, WebCore::Frame& frame, const std::optional<WebCore::NodeIdentifier>&)
{
    FramePrivate fp(&frame);
    DataTransferPrivate dt(&dataTransfer);
    m_appClient->startDrag(reinterpret_cast<WKC::DragImageRef>(item.image.get()), item.dragLocationInContentCoordinates, item.eventPositionInContentCoordinates, &dt.wkc(), &fp.wkc(), false);
}

} // namespace
