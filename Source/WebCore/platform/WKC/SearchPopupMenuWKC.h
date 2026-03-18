#pragma once

#include "PopupMenuWKC.h"
#include "SearchPopupMenu.h"

namespace WebCore {

class SearchPopupMenuWKC : public SearchPopupMenu {
public:
    SearchPopupMenuWKC(PopupMenuClient*);

    PopupMenu* popupMenu() override;
    void saveRecentSearches(const AtomString& name, const Vector<RecentSearch>& searchItems) override;
    void loadRecentSearches(const AtomString& name, Vector<RecentSearch>& searchItems) override;
    bool enabled() override;

private:
    Ref<PopupMenuWKC> m_popup;
};

} // namespace WebCore
