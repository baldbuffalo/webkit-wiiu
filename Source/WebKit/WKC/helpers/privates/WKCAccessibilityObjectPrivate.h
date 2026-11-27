/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC accessibility bridge.
 */

#pragma once

#include "helpers/WKCAccessibilityObject.h"
#include <wtf/Ref.h>
#include <wtf/RefCounted.h>
#include <WebCore/AXCoreObject.h>

namespace WKC {

class AccessibilityObjectPrivate : public RefCounted<AccessibilityObjectPrivate> {
public:
    static AccessibilityObject create(WebCore::AXCoreObject*);

    WebCore::AXCoreObject* webcore() const { return m_object.ptr(); }

private:
    explicit AccessibilityObjectPrivate(WebCore::AXCoreObject& object)
        : m_object(object)
    {
    }

    Ref<WebCore::AXCoreObject> m_object;

    friend class AccessibilityObject;
};

} // namespace WKC
