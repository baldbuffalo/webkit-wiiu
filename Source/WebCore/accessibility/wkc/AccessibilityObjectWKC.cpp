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
}

bool AccessibilityObject::accessibilityIgnoreAttachment() const
{
    return false;
}

AccessibilityObjectInclusion AccessibilityObject::accessibilityPlatformIncludesObject() const
{
    return AccessibilityObjectInclusion::DefaultBehavior;
}

} // namespace WebCore
