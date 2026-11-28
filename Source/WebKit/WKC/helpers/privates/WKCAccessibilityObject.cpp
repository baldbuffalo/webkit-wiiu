/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC accessibility bridge.
 */

#include "config.h"

#include "helpers/WKCAccessibilityObject.h"
#include "helpers/privates/WKCAccessibilityObjectPrivate.h"

#include "AccessibilityObject.h"

#include "FloatRect.h"
#include "IntPoint.h"

namespace WKC {

static String toWKCString(const WTF::String& value)
{
    auto utf8 = value.utf8();
    return String::fromUTF8(utf8.legacyCStringPointer(), utf8.length());
}

AccessibilityObject AccessibilityObjectPrivate::create(WebCore::AXCoreObject* object)
{
    if (!object)
        return { };

    Ref<AccessibilityObjectPrivate> privateObject = adoptRef(*new AccessibilityObjectPrivate(*object));
    return AccessibilityObject(privateObject.ptr());
}

AccessibilityObject::AccessibilityObject()
    : m_private(nullptr)
{
}

AccessibilityObject::AccessibilityObject(AccessibilityObjectPrivate* privateObject)
    : m_private(privateObject)
{
    if (m_private)
        m_private->ref();
}

AccessibilityObject::AccessibilityObject(const AccessibilityObject& other)
    : m_private(other.m_private)
{
    if (m_private)
        m_private->ref();
}

AccessibilityObject& AccessibilityObject::operator=(const AccessibilityObject& other)
{
    if (m_private == other.m_private)
        return *this;

    if (other.m_private)
        other.m_private->ref();
    if (m_private)
        m_private->deref();
    m_private = other.m_private;
    return *this;
}

AccessibilityObject::~AccessibilityObject()
{
    if (m_private)
        m_private->deref();
}

bool AccessibilityObject::isValid() const
{
    auto* object = m_private ? m_private->webcore() : nullptr;
    return object && !object->isDetached();
}

unsigned AccessibilityObject::role() const
{
    return isValid() ? static_cast<unsigned>(m_private->webcore()->role()) : 0;
}

String AccessibilityObject::roleDescription() const
{
    return isValid() ? toWKCString(m_private->webcore()->roleDescription()) : String();
}

String AccessibilityObject::ariaRoleDescription() const
{
    return isValid() ? toWKCString(m_private->webcore()->ariaRoleDescription()) : String();
}

String AccessibilityObject::currentValue() const
{
    return isValid() ? toWKCString(m_private->webcore()->currentValue()) : String();
}

String AccessibilityObject::popupValue() const
{
    return isValid() ? toWKCString(m_private->webcore()->popupValueString()) : String();
}

String AccessibilityObject::title() const
{
    return isValid() ? toWKCString(m_private->webcore()->title()) : String();
}

String AccessibilityObject::accessibleName() const
{
    if (!isValid())
        return { };

    auto* object = dynamicDowncast<WebCore::AccessibilityObject>(m_private->webcore());
    return toWKCString(object ? object->computedLabel() : m_private->webcore()->title());
}

String AccessibilityObject::announcementText() const
{
    if (!isValid())
        return { };

    auto* object = dynamicDowncast<WebCore::AccessibilityObject>(m_private->webcore());
    return toWKCString(object ? object->announcementText() : m_private->webcore()->text());
}

String AccessibilityObject::brailleLabel() const
{
    return isValid() ? toWKCString(m_private->webcore()->brailleLabel()) : String();
}

String AccessibilityObject::brailleRoleDescription() const
{
    return isValid() ? toWKCString(m_private->webcore()->brailleRoleDescription()) : String();
}

String AccessibilityObject::keyShortcuts() const
{
    return isValid() ? toWKCString(m_private->webcore()->keyShortcuts()) : String();
}

String AccessibilityObject::identifier() const
{
    return isValid() ? toWKCString(m_private->webcore()->identifierAttribute()) : String();
}

String AccessibilityObject::invalidStatus() const
{
    return isValid() ? toWKCString(m_private->webcore()->invalidStatusIncludingInferred()) : String();
}

String AccessibilityObject::description() const
{
    return isValid() ? toWKCString(m_private->webcore()->description()) : String();
}

String AccessibilityObject::text() const
{
    return isValid() ? toWKCString(m_private->webcore()->text()) : String();
}

String AccessibilityObject::value() const
{
    // Do not expose secure text input through the general accessibility value API.
    return isValid() && !m_private->webcore()->isSecureField() ? toWKCString(m_private->webcore()->stringValue()) : String();
}

String AccessibilityObject::selectedText() const
{
    // Never return selected characters from a secure text field.
    return isValid() && !m_private->webcore()->isSecureField() ? toWKCString(m_private->webcore()->selectedText()) : String();
}

unsigned AccessibilityObject::textLength() const
{
    // Do not disclose the character count of secure text fields.
    return isValid() && !m_private->webcore()->isSecureField() ? m_private->webcore()->textLength() : 0;
}

String AccessibilityObject::language() const
{
    return isValid() ? toWKCString(m_private->webcore()->language()) : String();
}

String AccessibilityObject::languageIncludingAncestors() const
{
    return isValid() ? toWKCString(m_private->webcore()->languageIncludingAncestors()) : String();
}

String AccessibilityObject::accessKey() const
{
    return isValid() ? toWKCString(m_private->webcore()->accessKey()) : String();
}

String AccessibilityObject::placeholderValue() const
{
    return isValid() ? toWKCString(m_private->webcore()->placeholderValue()) : String();
}

String AccessibilityObject::extendedDescription() const
{
    return isValid() ? toWKCString(m_private->webcore()->extendedDescription()) : String();
}

String AccessibilityObject::valueDescription() const
{
    return isValid() ? toWKCString(m_private->webcore()->valueDescription()) : String();
}

String AccessibilityObject::linkURL() const
{
    if (!isValid() || !m_private->webcore()->isLink())
        return { };

    auto url = m_private->webcore()->url();
    // A link's user-info is not useful to assistive navigation and can contain credentials.
    url.removeCredentials();
    return toWKCString(url.string());
}

String AccessibilityObject::linkRelation() const
{
    return isValid() && m_private->webcore()->isLink() ? toWKCString(m_private->webcore()->linkRelValue()) : String();
}

bool AccessibilityObject::isHeading() const
{
    return isValid() && m_private->webcore()->isHeading();
}

bool AccessibilityObject::isLink() const
{
    return isValid() && m_private->webcore()->isLink();
}

bool AccessibilityObject::isImage() const
{
    return isValid() && m_private->webcore()->isImage();
}

bool AccessibilityObject::isWebArea() const
{
    return isValid() && m_private->webcore()->isWebArea();
}

bool AccessibilityObject::isEditableWebArea() const
{
    return isValid() && m_private->webcore()->isEditableWebArea();
}

bool AccessibilityObject::isTextControl() const
{
    return isValid() && m_private->webcore()->isTextControl();
}

bool AccessibilityObject::isTable() const
{
    return isValid() && m_private->webcore()->isTable();
}

bool AccessibilityObject::isTableRow() const
{
    return isValid() && m_private->webcore()->isTableRow();
}

bool AccessibilityObject::isTableCell() const
{
    return isValid() && m_private->webcore()->isTableCell();
}

bool AccessibilityObject::isButton() const
{
    return isValid() && m_private->webcore()->isButton();
}

bool AccessibilityObject::isCheckbox() const
{
    return isValid() && m_private->webcore()->isCheckbox();
}

bool AccessibilityObject::isRadioButton() const
{
    return isValid() && m_private->webcore()->isRadioButton();
}

bool AccessibilityObject::isList() const
{
    return isValid() && m_private->webcore()->isList();
}

bool AccessibilityObject::isListItem() const
{
    return isValid() && m_private->webcore()->isListItem();
}

bool AccessibilityObject::isListBox() const
{
    return isValid() && m_private->webcore()->isListBox();
}

bool AccessibilityObject::isListBoxOption() const
{
    return isValid() && m_private->webcore()->isListBoxOption();
}

bool AccessibilityObject::isMenu() const
{
    return isValid() && m_private->webcore()->isMenu();
}

bool AccessibilityObject::isMenuBar() const
{
    return isValid() && m_private->webcore()->isMenuBar();
}

bool AccessibilityObject::isMenuItem() const
{
    return isValid() && m_private->webcore()->isMenuItem();
}

bool AccessibilityObject::isProgressIndicator() const
{
    return isValid() && m_private->webcore()->isProgressIndicator();
}

bool AccessibilityObject::isSlider() const
{
    return isValid() && m_private->webcore()->isSlider();
}

bool AccessibilityObject::isSpinButton() const
{
    return isValid() && m_private->webcore()->isSpinButton();
}

bool AccessibilityObject::isSwitch() const
{
    return isValid() && m_private->webcore()->isSwitch();
}

bool AccessibilityObject::isToggleButton() const
{
    return isValid() && m_private->webcore()->isToggleButton();
}

bool AccessibilityObject::isTabList() const
{
    return isValid() && m_private->webcore()->isTabList();
}

bool AccessibilityObject::isTabItem() const
{
    return isValid() && m_private->webcore()->isTabItem();
}

bool AccessibilityObject::isRadioGroup() const
{
    return isValid() && m_private->webcore()->isRadioGroup();
}

bool AccessibilityObject::isComboBox() const
{
    return isValid() && m_private->webcore()->isComboBox();
}

bool AccessibilityObject::isDateTime() const
{
    return isValid() && m_private->webcore()->isDateTime();
}

bool AccessibilityObject::isGrid() const
{
    return isValid() && m_private->webcore()->isGrid();
}

bool AccessibilityObject::isTree() const
{
    return isValid() && m_private->webcore()->isTree();
}

bool AccessibilityObject::isTreeGrid() const
{
    return isValid() && m_private->webcore()->isTreeGrid();
}

bool AccessibilityObject::isTreeItem() const
{
    return isValid() && m_private->webcore()->isTreeItem();
}

bool AccessibilityObject::isToolbar() const
{
    return isValid() && m_private->webcore()->isToolbar();
}

bool AccessibilityObject::isLandmark() const
{
    return isValid() && m_private->webcore()->isLandmark();
}

bool AccessibilityObject::isMeter() const
{
    return isValid() && m_private->webcore()->isMeter();
}

bool AccessibilityObject::isPopUpButton() const
{
    return isValid() && m_private->webcore()->isPopUpButton();
}

bool AccessibilityObject::isScrollArea() const
{
    return isValid() && m_private->webcore()->isScrollArea();
}

bool AccessibilityObject::isCanvas() const
{
    return isValid() && m_private->webcore()->isCanvas();
}

bool AccessibilityObject::isKeyboardFocusable() const
{
    return isValid() && m_private->webcore()->isKeyboardFocusable();
}

bool AccessibilityObject::canSetFocus() const
{
    return isValid() && m_private->webcore()->canSetFocusAttribute();
}

bool AccessibilityObject::isOnScreen() const
{
    return isValid() && m_private->webcore()->isOnScreen();
}

bool AccessibilityObject::isLoaded() const
{
    return isValid() && m_private->webcore()->isLoaded();
}

bool AccessibilityObject::isBusy() const
{
    return isValid() && m_private->webcore()->isBusy();
}

bool AccessibilityObject::isARIAHidden() const
{
    return isValid() && m_private->webcore()->isARIAHidden();
}

bool AccessibilityObject::isOutput() const
{
    return isValid() && m_private->webcore()->isOutput();
}

bool AccessibilityObject::supportsRangeValue() const
{
    return isValid() && m_private->webcore()->supportsRangeValue();
}

float AccessibilityObject::rangeValue() const
{
    return supportsRangeValue() ? m_private->webcore()->valueForRange() : 0.0f;
}

float AccessibilityObject::minimumRangeValue() const
{
    return supportsRangeValue() ? m_private->webcore()->minValueForRange() : 0.0f;
}

float AccessibilityObject::maximumRangeValue() const
{
    return supportsRangeValue() ? m_private->webcore()->maxValueForRange() : 0.0f;
}

unsigned AccessibilityObject::headingLevel() const
{
    return isHeading() ? m_private->webcore()->headingLevel() : 0;
}

unsigned AccessibilityObject::ariaLevel() const
{
    return isValid() ? m_private->webcore()->ariaLevel() : 0;
}

bool AccessibilityObject::isIgnored() const
{
    return isValid() && m_private->webcore()->isIgnored();
}

bool AccessibilityObject::isEnabled() const
{
    return isValid() && m_private->webcore()->isEnabled();
}

bool AccessibilityObject::isSelected() const
{
    return isValid() && m_private->webcore()->isSelected();
}

bool AccessibilityObject::isChecked() const
{
    return isValid() && m_private->webcore()->isChecked();
}

bool AccessibilityObject::isFocused() const
{
    return isValid() && m_private->webcore()->isFocused();
}

bool AccessibilityObject::isExpanded() const
{
    return isValid() && m_private->webcore()->isExpanded();
}

bool AccessibilityObject::isVisible() const
{
    return isValid() && m_private->webcore()->isVisible();
}

bool AccessibilityObject::isIndeterminate() const
{
    return isValid() && m_private->webcore()->isIndeterminate();
}

bool AccessibilityObject::isMultiSelectable() const
{
    return isValid() && m_private->webcore()->isMultiSelectable();
}

bool AccessibilityObject::isOffScreen() const
{
    return isValid() && m_private->webcore()->isOffScreen();
}

bool AccessibilityObject::isRequired() const
{
    return isValid() && m_private->webcore()->isRequired();
}

bool AccessibilityObject::isSecureField() const
{
    return isValid() && m_private->webcore()->isSecureField();
}

bool AccessibilityObject::canSetValue() const
{
    return isValid() && m_private->webcore()->canSetValueAttribute();
}

bool AccessibilityObject::supportsPressAction() const
{
    return isValid() && m_private->webcore()->supportsPressAction();
}

bool AccessibilityObject::isPressed() const
{
    return isValid() && m_private->webcore()->isPressed();
}

bool AccessibilityObject::isVisited() const
{
    return isValid() && m_private->webcore()->isVisited();
}

bool AccessibilityObject::isModal() const
{
    return isValid() && m_private->webcore()->isModalNode();
}

bool AccessibilityObject::hasPopup() const
{
    return isValid() && m_private->webcore()->hasPopup();
}

int AccessibilityObject::posInSet() const
{
    return isValid() ? m_private->webcore()->posInSet() : 0;
}

int AccessibilityObject::setSize() const
{
    return isValid() ? m_private->webcore()->setSize() : 0;
}

WKCFloatRect AccessibilityObject::frame() const
{
    if (!isValid())
        return { { 0, 0 }, { 0, 0 } };

    auto rect = m_private->webcore()->relativeFrame();
    return { { rect.x(), rect.y() }, { rect.width(), rect.height() } };
}

unsigned AccessibilityObject::childCount() const
{
    if (!isValid())
        return 0;
    return m_private->webcore()->unignoredChildren().size();
}

AccessibilityObject AccessibilityObject::childAt(unsigned index) const
{
    if (!isValid())
        return { };

    const auto& children = m_private->webcore()->unignoredChildren();
    if (index >= children.size())
        return { };
    return AccessibilityObjectPrivate::create(children[index].ptr());
}

AccessibilityObject AccessibilityObject::parent() const
{
    if (!isValid())
        return { };
    return AccessibilityObjectPrivate::create(m_private->webcore()->parentObjectUnignored());
}

AccessibilityObject AccessibilityObject::nextSibling() const
{
    return isValid() ? AccessibilityObjectPrivate::create(m_private->webcore()->nextSiblingUnignored()) : AccessibilityObject { };
}

AccessibilityObject AccessibilityObject::previousSibling() const
{
    return isValid() ? AccessibilityObjectPrivate::create(m_private->webcore()->previousSiblingUnignored()) : AccessibilityObject { };
}

unsigned AccessibilityObject::documentLinkCount() const
{
    return isValid() ? m_private->webcore()->documentLinks().size() : 0;
}

AccessibilityObject AccessibilityObject::documentLinkAt(unsigned index) const
{
    if (!isValid())
        return { };

    auto links = m_private->webcore()->documentLinks();
    if (index >= links.size())
        return { };

    return AccessibilityObjectPrivate::create(links[index].ptr());
}

void AccessibilityObject::scrollToMakeVisible() const
{
    if (isValid())
        m_private->webcore()->scrollToMakeVisible();
}

AccessibilityObject AccessibilityObject::hitTest(int x, int y) const
{
    if (!isValid())
        return { };

    auto hitObject = m_private->webcore()->accessibilityHitTest(WebCore::IntPoint(x, y));
    return AccessibilityObjectPrivate::create(hitObject.get());
}

unsigned AccessibilityObject::rowCount() const
{
    return isTable() ? m_private->webcore()->rowCount() : 0;
}

unsigned AccessibilityObject::columnCount() const
{
    return isTable() ? m_private->webcore()->columnCount() : 0;
}

unsigned AccessibilityObject::rowIndex() const
{
    return isTableRow() ? m_private->webcore()->rowIndex() : 0;
}

unsigned AccessibilityObject::columnIndex() const
{
    return isTableCell() ? m_private->webcore()->columnIndex() : 0;
}

AccessibilityObject AccessibilityObject::cellForColumnAndRow(unsigned column, unsigned row) const
{
    if (!isTable())
        return { };

    return AccessibilityObjectPrivate::create(m_private->webcore()->cellForColumnAndRow(column, row));
}

AccessibilityObject AccessibilityObject::rowHeader() const
{
    if (!isTableCell())
        return { };

    return AccessibilityObjectPrivate::create(m_private->webcore()->rowHeader());
}

AccessibilityObject AccessibilityObject::columnHeader() const
{
    if (!isTableCell())
        return { };

    return AccessibilityObjectPrivate::create(m_private->webcore()->columnHeader());
}

static WebCore::AXRelation toWebCoreRelation(AccessibilityObject::Relation relation)
{
    switch (relation) {
    case AccessibilityObject::Relation::None: return WebCore::AXRelation::None;
    case AccessibilityObject::Relation::Actions: return WebCore::AXRelation::Actions;
    case AccessibilityObject::Relation::ActionsOf: return WebCore::AXRelation::ActionsOf;
    case AccessibilityObject::Relation::ActiveDescendant: return WebCore::AXRelation::ActiveDescendant;
    case AccessibilityObject::Relation::ActiveDescendantOf: return WebCore::AXRelation::ActiveDescendantOf;
    case AccessibilityObject::Relation::ControlledBy: return WebCore::AXRelation::ControlledBy;
    case AccessibilityObject::Relation::ControllerFor: return WebCore::AXRelation::ControllerFor;
    case AccessibilityObject::Relation::DescribedBy: return WebCore::AXRelation::DescribedBy;
    case AccessibilityObject::Relation::DescriptionFor: return WebCore::AXRelation::DescriptionFor;
    case AccessibilityObject::Relation::Details: return WebCore::AXRelation::Details;
    case AccessibilityObject::Relation::DetailsFor: return WebCore::AXRelation::DetailsFor;
    case AccessibilityObject::Relation::ErrorMessage: return WebCore::AXRelation::ErrorMessage;
    case AccessibilityObject::Relation::ErrorMessageFor: return WebCore::AXRelation::ErrorMessageFor;
    case AccessibilityObject::Relation::FlowsFrom: return WebCore::AXRelation::FlowsFrom;
    case AccessibilityObject::Relation::FlowsTo: return WebCore::AXRelation::FlowsTo;
    case AccessibilityObject::Relation::Headers: return WebCore::AXRelation::Headers;
    case AccessibilityObject::Relation::HeaderFor: return WebCore::AXRelation::HeaderFor;
    case AccessibilityObject::Relation::LabeledBy: return WebCore::AXRelation::LabeledBy;
    case AccessibilityObject::Relation::LabelFor: return WebCore::AXRelation::LabelFor;
    case AccessibilityObject::Relation::NativeLabeledBy: return WebCore::AXRelation::NativeLabeledBy;
    case AccessibilityObject::Relation::NativeLabelFor: return WebCore::AXRelation::NativeLabelFor;
    case AccessibilityObject::Relation::OwnedBy: return WebCore::AXRelation::OwnedBy;
    case AccessibilityObject::Relation::OwnerFor: return WebCore::AXRelation::OwnerFor;
    }
    return WebCore::AXRelation::None;
}

unsigned AccessibilityObject::relatedObjectCount(Relation relation) const
{
    return isValid() ? m_private->webcore()->relatedObjects(toWebCoreRelation(relation)).size() : 0;
}

AccessibilityObject AccessibilityObject::relatedObjectAt(Relation relation, unsigned index) const
{
    if (!isValid())
        return { };

    auto objects = m_private->webcore()->relatedObjects(toWebCoreRelation(relation));
    if (index >= objects.size())
        return { };

    return AccessibilityObjectPrivate::create(objects[index].ptr());
}

bool AccessibilityObject::press() const
{
    return isValid() && m_private->webcore()->press();
}

bool AccessibilityObject::setValue(const String& value) const
{
    return isValid() && m_private->webcore()->setValue(WTF::String(value));
}

void AccessibilityObject::setFocused(bool focused) const
{
    if (isValid())
        m_private->webcore()->setFocused(focused);
}

void AccessibilityObject::setSelected(bool selected) const
{
    if (isValid())
        m_private->webcore()->setSelected(selected);
}

void AccessibilityObject::setSelectedText(const String& text) const
{
    if (isValid() && m_private->webcore()->canSetTextRangeAttributes())
        m_private->webcore()->setSelectedText(WTF::String(text));
}

void AccessibilityObject::setExpanded(bool expanded) const
{
    if (isValid() && m_private->webcore()->canSetExpandedAttribute())
        m_private->webcore()->setIsExpanded(expanded);
}

} // namespace WKC
