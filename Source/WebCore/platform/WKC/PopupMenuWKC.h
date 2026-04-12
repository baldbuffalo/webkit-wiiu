#pragma once

#include <wtf/RefCounted.h>
#include "IntRect.h"
#include "PopupMenu.h"
#include "helpers/privates/WKCPopupMenuClientPrivate.h"

namespace WebCore {
class LocalFrameView;
class PopupMenuClient;

class PopupMenuWKC : public PopupMenu {
public:
    PopupMenuWKC(PopupMenuClient*);
    ~PopupMenuWKC() override;
    void show(const IntRect&, LocalFrameView&, int selectedIndex) override;
    void hide() override;
    void updateFromElement() override;
    void disconnectClient() override;
private:
    PopupMenuClient* client() const { return m_popupClient; }
    PopupMenuClient* m_popupClient;
    WKC::PopupMenuClientPrivate* m_wkc;
    bool m_visible;
};
} // namespace WebCore
