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

#include "helpers/WKCFocusController.h"
#include "helpers/privates/WKCFocusControllerPrivate.h"

#include "FocusController.h"
#include "IntRect.h"

#include "SpatialNavigation.h"

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"

namespace {

static bool
convertFocusDir(const WKC::FocusDirection wkc_dir, WebCore::FocusDirection& webcore_dir)
{
    switch (wkc_dir) {
        case WKC::EFocusDirectionUp:
            webcore_dir = WebCore::FocusDirection::Up;
            break;
        case WKC::EFocusDirectionDown:
            webcore_dir = WebCore::FocusDirection::Down;
            break;
        case WKC::EFocusDirectionLeft:
            webcore_dir = WebCore::FocusDirection::Left;
            break;
        case WKC::EFocusDirectionRight:
            webcore_dir = WebCore::FocusDirection::Right;
            break;
        default:
            return false;
    }
    return true;
}

}
namespace WKC {

FocusControllerPrivate::FocusControllerPrivate(WebCore::FocusController* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_focusedFrame(0)
    , m_focusableNode(0)
    , m_focusableNodeInRect(0)
{
}

FocusControllerPrivate::~FocusControllerPrivate()
{
    delete m_focusableNodeInRect;
    delete m_focusableNode;
    delete m_focusedFrame;
}

Frame*
FocusControllerPrivate::focusedOrMainFrame()
{
    WebCore::Frame* frame = m_webcore->focusedOrMainFrame();
    if (!frame)
        return 0;
    if (!m_focusedFrame || m_focusedFrame->webcore()!=frame) {
        delete m_focusedFrame;
        m_focusedFrame = new FramePrivate(frame);
    }
    return &m_focusedFrame->wkc();
}

Node*
FocusControllerPrivate::findNextFocusableNode(FocusDirection direction, const WKCRect* specificRect)
{
    WebCore::FocusDirection webcore_dir;
    bool ok = convertFocusDir(direction, webcore_dir);
    if (!ok) {
        return 0;
    }

    // Spatial navigation (findNextFocusableNode) was an ACCESS extension to
    // WebCore::FocusController that no longer exists upstream. Degrade to "no
    // spatial-nav candidate" so the WKC API stays available; regular
    // tab-order focus is unaffected.
    (void)webcore_dir;
    WebCore::Node* node = nullptr;
    if (!node)
        return 0;

    if (m_focusableNode)
        delete m_focusableNode;
    m_focusableNode = NodePrivate::create(node);
    if (!m_focusableNode)
        return 0;

    return &m_focusableNode->wkc();
}

Node*
#if PLATFORM(WKC)
FocusControllerPrivate::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect, bool enableContainer)
#else
FocusControllerPrivate::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect)
#endif
{
    WebCore::FocusDirection webcore_dir;
    bool ok = convertFocusDir(direction, webcore_dir);
    if (!ok) {
        return 0;
    }
    // findNextFocusableNodeInRect was an ACCESS spatial-navigation extension
    // to WebCore::FocusController, removed upstream. Degrade to no candidate.
    (void)webcore_dir;
    (void)frame;
    (void)rect;
#if PLATFORM(WKC)
    (void)enableContainer;
#endif
    WebCore::Node* node = nullptr;
    if (!node)
        return 0;

    if (m_focusableNodeInRect)
        delete m_focusableNodeInRect;
    m_focusableNodeInRect = NodePrivate::create(node);
    if (!m_focusableNodeInRect)
        return 0;

    return &m_focusableNodeInRect->wkc();
}

Node*
FocusControllerPrivate::findNearestFocusableNodeFromPoint(const WKCPoint point, const WKCRect* rect)
{
    // findNearestFocusableNodeFromPoint was an ACCESS spatial-navigation
    // extension to WebCore::FocusController, removed upstream. Degrade to no
    // candidate.
    (void)point;
    (void)rect;
    WebCore::Node* node = nullptr;
    if( !node )
        return 0;

    if (m_focusableNodeInRect)
        delete m_focusableNodeInRect;
    m_focusableNodeInRect = NodePrivate::create(node);
    if (!m_focusableNodeInRect)
        return 0;

    return &m_focusableNodeInRect->wkc();
}

bool
isScrollableContainerNode(Node* node)
{
    if (!node)
        return false;

    WebCore::FocusCandidate fc(node->priv().webcore(), WebCore::FocusDirection::None);

    return fc.inScrollableContainer();
}

bool
hasOffscreenRect(Node* node)
{
    return WebCore::hasOffscreenRect(node ? node->priv().webcore() : 0);
}


FocusController::FocusController(FocusControllerPrivate& parent)
    : m_private(parent)
{
}

FocusController::~FocusController()
{
}

Frame*
FocusController::focusedOrMainFrame()
{
    return m_private.focusedOrMainFrame();
}

Node* FocusController::findNextFocusableNode(const FocusDirection direction, const WKCRect* specificRect)
{
    return m_private.findNextFocusableNode(direction, specificRect);
}

Node*
FocusController::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect, bool enableContainer)
{
    return m_private.findNextFocusableNodeInRect(direction, frame, rect, enableContainer);
}

Node*
FocusController::findNearestFocusableNodeFromPoint(const WKCPoint point, const WKCRect* rect)
{
    return m_private.findNearestFocusableNodeFromPoint(point, rect);
}

} // namespace
