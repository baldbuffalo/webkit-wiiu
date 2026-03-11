/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007, Holger Hans Peter Freyther
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "DataTransfer.h"
#include <wtf/URL.h>

namespace WebCore {

class CachedImage;
class DragData;
class FileList;
class Frame;
class DataTransferItemList;

class ClipboardWKC : public DataTransfer {
public:
    static Ref<ClipboardWKC> create(DataTransfer::Type type, DataTransferAccessPolicy policy, DragData* data, Frame* frame)
    {
        return adoptRef(*new ClipboardWKC(type, policy, data, frame));
    }
    virtual ~ClipboardWKC();

    void clearData(const String&);
    void clearAllData();
    String getData(const String&) const;
    bool setData(const String&, const String&);

    HashSet<String> types() const override;
    RefPtr<FileList> files() const override;

    IntPoint dragLocation() const;
    CachedImage* dragImage() const;
    void setDragImage(CachedImage*, const IntPoint&);
    Node* dragImageElement();
    void setDragImageElement(Node*, const IntPoint&);

    DragImageRef createDragImage(IntPoint&) const override;
    void declareAndWriteDragImage(Element*, const URL&, const String&, Frame*) override;
    void writeURL(const URL&, const String&, Frame*) override;
    void writeRange(Range*, Frame*) override;
    void writePlainText(const String&) override;

    bool hasData() override;
    RefPtr<DataTransferItemList> items() override;

private:
    ClipboardWKC(DataTransfer::Type, DataTransferAccessPolicy, DragData*, Frame*);

    DragData* m_data;
    Frame* m_frame;
};

} // namespace WebCore
