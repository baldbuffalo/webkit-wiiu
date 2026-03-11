/*
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *  Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#if ENABLE(CONTEXT_MENUS)

#include "ContextMenu.h"
#include "ContextMenuItem.h"
#include "NotImplemented.h"

namespace WebCore {

ContextMenuItem::ContextMenuItem(ContextMenuItemType type, ContextMenuAction action, const String& title, ContextMenu* subMenu)
    : m_type(type)
    , m_action(action)
    , m_title(title)
    , m_enabled(true)
    , m_checked(false)
{
}

ContextMenuItem::~ContextMenuItem()
{
}

ContextMenuItemType ContextMenuItem::type() const
{
    return m_type;
}

void ContextMenuItem::setType(ContextMenuItemType type)
{
    m_type = type;
}

ContextMenuAction ContextMenuItem::action() const
{
    return m_action;
}

void ContextMenuItem::setAction(ContextMenuAction action)
{
    m_action = action;
}

String ContextMenuItem::title() const
{
    return m_title;
}

void ContextMenuItem::setTitle(const String& title)
{
    m_title = title;
}

void ContextMenuItem::setSubMenu(ContextMenu* menu)
{
    notImplemented();
}

void ContextMenuItem::setChecked(bool shouldCheck)
{
    m_checked = shouldCheck;
}

void ContextMenuItem::setEnabled(bool shouldEnable)
{
    m_enabled = shouldEnable;
}

} // namespace WebCore

#endif // ENABLE(CONTEXT_MENUS)
