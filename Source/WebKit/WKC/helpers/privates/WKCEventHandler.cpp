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

#include "helpers/WKCEventHandler.h"
#include "helpers/privates/WKCEventHandlerPrivate.h"

#include "EventHandler.h"
#include "Frame.h"
#include "LocalFrame.h"

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"

#include "wkcglobalwrapper.h"

namespace WKC {

Frame*
EventHandlerPrivate::subframeForTargetNode(Node* node)
{
    WebCore::Node* n = node ? node->priv().webcore() : nullptr;

    RefPtr<WebCore::Frame> f = WebCore::EventHandler::subframeForTargetNode(n);
    if (!f)
        return nullptr;

    // The cached FramePrivate wrapper is registered with the engine's
    // global-object-reset peer via WKC_DEFINE_STATIC_PTR, so it must stay a
    // raw pointer with manual lifetime management (a smart pointer would not
    // cooperate with the peer that zeroes the static on engine reset).
    WKC_DEFINE_STATIC_PTR(FramePrivate*, gFrame, nullptr);
    if (!gFrame || gFrame->webcore() != f.get()) {
        delete gFrame;
        gFrame = new FramePrivate(f.get());
    }
    return &gFrame->wkc();
}

Frame*
EventHandler::subframeForTargetNode(Node* node)
{
    return EventHandlerPrivate::subframeForTargetNode(node);
}

} // namespace
