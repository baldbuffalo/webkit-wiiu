/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC WebCore accessibility backend.
 */

#include "config.h"

#include "AXObjectCache.h"

#include "AXCoreObject.h"
#include "AXNotifications.h"
#include "AccessibilityObject.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "DocumentPage.h"
#include "DocumentView.h"
#include "FrameDestructionObserverInlines.h"
#include "HTMLSelectElement.h"
#include "LocalFrame.h"
#include "NodeDocument.h"
#include "Page.h"
#include "RenderView.h"

namespace WebCore {

void AXObjectCache::attachWrapper(AccessibilityObject& object)
{
    auto wrapper = adoptRef(*new AccessibilityObjectWrapper(object));
    object.setWrapper(wrapper.ptr());
}

void AXObjectCache::detachWrapper(AXCoreObject* object, AccessibilityDetachmentType)
{
    if (object)
        object->setWrapper(nullptr);
}

static AXCoreObject* notifyChildrenSelectionChange(AXCoreObject* object)
{
    if (!object || !object->isListBox())
        return object;

    Node* node = object->node();
    if (!is<HTMLSelectElement>(node))
        return object;

    auto& select = downcast<HTMLSelectElement>(*node);
    int changedItemIndex = select.activeSelectionStartListIndex();
    const auto& items = object->children();
    if (changedItemIndex < 0 || changedItemIndex >= static_cast<int>(items.size()))
        return object;
    return items.at(changedItemIndex).ptr();
}

static AXNotification checkInteractableObjects(AXCoreObject& object)
{
    if (!object.isEnabled())
        return AXNotification::PressDidFail;

    if (object.isTextControl() && !object.canSetValueAttribute())
        return AXNotification::PressDidFail;

    return AXNotification::PressDidSucceed;
}

void AXObjectCache::postPlatformNotification(AccessibilityObject& object, AXNotification notification)
{
    if (!document()
        || !object.document()
        || !object.document()->view()
        || object.document()->view()->layoutContext().layoutState()
        || object.document()->childNeedsStyleRecalc())
        return;

    RefPtr protectedObject = object;
    switch (notification) {
    case AXNotification::SelectedChildrenChanged:
        protectedObject = downcast<AccessibilityObject>(*notifyChildrenSelectionChange(protectedObject.get()));
        break;
    case AXNotification::PressDidSucceed:
        notification = checkInteractableObjects(*protectedObject);
        break;
    default:
        break;
    }

    auto* frame = document()->frame();
    if (!frame || !frame->page())
        return;
    frame->page()->chrome().client().postAccessibilityNotification(*protectedObject, notification);
}

void AXObjectCache::nodeTextChangePlatformNotification(AccessibilityObject* object, AXTextChange textChange, unsigned offset, const String& text)
{
    if (!document()
        || !object
        || !object->document()
        || !object->document()->view()
        || object->document()->view()->layoutContext().layoutState()
        || object->document()->childNeedsStyleRecalc())
        return;

    auto* frame = document()->frame();
    if (!frame || !frame->page())
        return;
    frame->page()->chrome().client().postAccessibilityNodeTextChangeNotification(object, textChange, offset, text);
}

void AXObjectCache::frameLoadingEventPlatformNotification(RenderView* renderView, AXLoadingEvent loadingEvent)
{
    if (!renderView || !document())
        return;

    RefPtr object = getOrCreate(*renderView);
    if (!object
        || !object->document()
        || !object->document()->view()
        || object->document()->view()->layoutContext().layoutState()
        || object->document()->childNeedsStyleRecalc())
        return;

    auto* frame = document()->frame();
    if (!frame || !frame->page())
        return;
    frame->page()->chrome().client().postAccessibilityFrameLoadingEventNotification(object.get(), loadingEvent);
}

void AXObjectCache::handleScrolledToAnchor(const Node& scrolledToNode)
{
    if (RefPtr object = AccessibilityObject::firstAccessibleObjectFromNode(&scrolledToNode))
        postPlatformNotification(*object, AXNotification::ScrolledToAnchor);
}

void AXObjectCache::platformHandleFocusedUIElementChanged(AccessibilityObject*, AccessibilityObject* newFocus)
{
    if (!newFocus)
        return;

    Page* page = newFocus->page();
    if (!page || !page->chrome().platformPageClient())
        return;

    if (RefPtr focusedObject = focusedObjectForPage(page))
        postPlatformNotification(*focusedObject, AXNotification::FocusedUIElementChanged);
}

void AXObjectCache::platformPerformDeferredCacheUpdate()
{
    // WKC currently exposes the live tree on WebCore's main thread; there is no
    // separate platform accessibility thread or deferred wrapper cache to flush.
}

} // namespace WebCore
