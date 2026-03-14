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

ContextMenu::ContextMenu()
{
}

void ContextMenu::appendItem(const ContextMenuItem& item)
{
    m_items.append(item);
}

} // namespace WebCore

#endif // ENABLE(CONTEXT_MENUS)
