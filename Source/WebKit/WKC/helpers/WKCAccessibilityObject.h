/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC accessibility bridge.
 */

#pragma once

#include <wkc/wkcbase.h>
#include "helpers/WKCString.h"

namespace WKC {

class AccessibilityObjectPrivate;

// A retained handle to a WebCore accessibility-tree object. Handles can be
// copied to keep an object alive across callbacks.
class WKC_API AccessibilityObject {
public:
    AccessibilityObject();
    AccessibilityObject(const AccessibilityObject&);
    AccessibilityObject& operator=(const AccessibilityObject&);
    ~AccessibilityObject();

    bool isValid() const;
    unsigned role() const;
    String roleDescription() const;
    String title() const;
    String description() const;
    String text() const;
    String value() const;

    bool isIgnored() const;
    bool isEnabled() const;
    bool isSelected() const;
    bool isChecked() const;
    bool isFocused() const;
    bool isExpanded() const;
    bool isVisible() const;

    // Bounds are relative to the current viewport, in WebCore view coordinates.
    WKCFloatRect frame() const;

    unsigned childCount() const;
    AccessibilityObject childAt(unsigned index) const;
    AccessibilityObject parent() const;

    bool press() const;
    bool setValue(const String&) const;
    void setFocused(bool) const;
    void setSelected(bool) const;

private:
    explicit AccessibilityObject(AccessibilityObjectPrivate*);
    friend class AccessibilityObjectPrivate;

    AccessibilityObjectPrivate* m_private;
};

} // namespace WKC
