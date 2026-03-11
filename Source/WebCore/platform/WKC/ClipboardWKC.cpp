/*
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "ClipboardWKC.h"

#include "CachedImage.h"
#include "DragData.h"
#include "Editor.h"
#include "Element.h"
#include "LocalFrame.h"
#include "FileList.h"
#include "markup.h"
#include "NotImplemented.h"
#include "RenderImage.h"
#include "DataTransferItemList.h"
#include <wtf/URL.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

ClipboardWKC::ClipboardWKC(DataTransfer::Type type, DataTransferAccessPolicy policy, DragData* data, LocalFrame* frame)
    : DataTransfer(policy, type)
    , m_data(data)
    , m_frame(frame)
{
}

ClipboardWKC::~ClipboardWKC()
{
}

void ClipboardWKC::clearData(const String&)
{
    notImplemented();
}

void ClipboardWKC::clearAllData()
{
    notImplemented();
}

String ClipboardWKC::getData(const String&) const
{
    notImplemented();
    return String();
}

bool ClipboardWKC::setData(const String&, const String&)
{
    notImplemented();
    return false;
}

Vector<String> ClipboardWKC::types() const
{
    notImplemented();
    return { };
}

RefPtr<FileList> ClipboardWKC::files() const
{
    notImplemented();
    return nullptr;
}

IntPoint ClipboardWKC::dragLocation() const
{
    notImplemented();
    return IntPoint(0, 0);
}

CachedImage* ClipboardWKC::dragImage() const
{
    notImplemented();
    return nullptr;
}

void ClipboardWKC::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}

Node* ClipboardWKC::dragImageElement()
{
    notImplemented();
    return nullptr;
}

void ClipboardWKC::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

DragImageRef ClipboardWKC::createDragImage(IntPoint&) const
{
    notImplemented();
    return nullptr;
}

void ClipboardWKC::declareAndWriteDragImage(Element& element, const URL& url, const String& label)
{
    notImplemented();
}

void ClipboardWKC::writeURL(const URL& url, const String& label, LocalFrame*)
{
    notImplemented();
}

void ClipboardWKC::writeRange(Range& range, LocalFrame& frame)
{
    notImplemented();
}

void ClipboardWKC::writePlainText(const String&)
{
    notImplemented();
}

bool ClipboardWKC::hasData()
{
    notImplemented();
    return false;
}

} // namespace WebCore
