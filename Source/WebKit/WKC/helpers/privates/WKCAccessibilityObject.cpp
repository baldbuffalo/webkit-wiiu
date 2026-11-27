/*
 * Copyright (C) 2026 OpenAI. All rights reserved.
 *
 * This file is part of the WKC accessibility bridge.
 */

#include "config.h"

#include "helpers/WKCAccessibilityObject.h"
#include "helpers/privates/WKCAccessibilityObjectPrivate.h"

#include "FloatRect.h"

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

String AccessibilityObject::title() const
{
    return isValid() ? toWKCString(m_private->webcore()->title()) : String();
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
    return isValid() ? toWKCString(m_private->webcore()->stringValue()) : String();
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

} // namespace WKC
