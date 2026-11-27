/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC WebCore accessibility backend.
 */

#include "config.h"

#include "AccessibilityObject.h"

namespace WebCore {

void AccessibilityObject::detachPlatformWrapper(AccessibilityDetachmentType)
{
    // WKC has no OS-owned accessibility peer to detach. AXCoreObject::detachWrapper()
    // releases the WebCore wrapper immediately after this hook returns.
}

bool AccessibilityObject::accessibilityIgnoreAttachment() const
{
    // WKC exposes embedded content through the same semantic tree as other objects.
    return false;
}

AccessibilityObjectInclusion AccessibilityObject::accessibilityPlatformIncludesObject() const
{
    // Let WebCore's common inclusion rules decide; WKC adds no platform-specific filtering.
    return AccessibilityObjectInclusion::DefaultBehavior;
}

} // namespace WebCore
