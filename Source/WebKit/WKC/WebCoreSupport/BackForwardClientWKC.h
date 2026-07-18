/*
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef BackForwardClientWKC_h
#define BackForwardClientWKC_h

#include "BackForwardClient.h"

namespace WebCore {
class HistoryItem;
}

namespace WKC {
class BackForwardClientIf;
class WKCWebViewPrivate;

class BackForwardClientWKC : public WebCore::BackForwardClient
{
public:
    static Ref<BackForwardClientWKC> create(WKCWebViewPrivate* view);
    virtual ~BackForwardClientWKC();

    void addItem(Ref<WebCore::HistoryItem>&&) override;
    void setChildItem(WebCore::BackForwardFrameItemIdentifier, Ref<WebCore::HistoryItem>&&) override;

    void goToItem(WebCore::HistoryItem&) override;

    Vector<Ref<WebCore::HistoryItem>> allItems(WebCore::FrameIdentifier) override;
    RefPtr<WebCore::HistoryItem> itemAtIndex(int, WebCore::FrameIdentifier) override;

    unsigned backListCount() const override;
    unsigned forwardListCount() const override;
    bool containsItem(const WebCore::HistoryItem&) const override;

    void close() override;

private:
    BackForwardClientWKC(WKCWebViewPrivate* view);
    bool construct();

private:
    WKCWebViewPrivate* m_view;
    WKC::BackForwardClientIf* m_appClient;
};

} // namespace

#endif // BackForwardClientWKC_h
