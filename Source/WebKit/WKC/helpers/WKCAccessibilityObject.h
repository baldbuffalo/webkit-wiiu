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
    enum class Relation : unsigned {
        None,
        Actions,
        ActionsOf,
        ActiveDescendant,
        ActiveDescendantOf,
        ControlledBy,
        ControllerFor,
        DescribedBy,
        DescriptionFor,
        Details,
        DetailsFor,
        ErrorMessage,
        ErrorMessageFor,
        FlowsFrom,
        FlowsTo,
        Headers,
        HeaderFor,
        LabeledBy,
        LabelFor,
        NativeLabeledBy,
        NativeLabelFor,
        OwnedBy,
        OwnerFor
    };

    AccessibilityObject();
    AccessibilityObject(const AccessibilityObject&);
    AccessibilityObject& operator=(const AccessibilityObject&);
    ~AccessibilityObject();

    bool isValid() const;
    unsigned role() const;
    String roleDescription() const;
    String ariaRoleDescription() const;
    String currentValue() const;
    String popupValue() const;
    String title() const;
    // Computed accessible name (ARIA, native labels, alt text and contents).
    String accessibleName() const;
    // Text to announce for this object, including ARIA live-region content.
    String announcementText() const;
    String keyShortcuts() const;
    String identifier() const;
    String invalidStatus() const;
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
    bool isIndeterminate() const;
    bool isMultiSelectable() const;
    bool isOffScreen() const;
    bool isRequired() const;
    bool isSecureField() const;
    bool canSetValue() const;
    bool supportsPressAction() const;
    bool isPressed() const;
    bool isVisited() const;
    bool isModal() const;
    bool hasPopup() const;
    int posInSet() const;
    int setSize() const;

    // Bounds are relative to the current viewport, in WebCore view coordinates.
    WKCFloatRect frame() const;

    unsigned childCount() const;
    AccessibilityObject childAt(unsigned index) const;
    AccessibilityObject parent() const;
    unsigned relatedObjectCount(Relation) const;
    AccessibilityObject relatedObjectAt(Relation, unsigned index) const;

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
