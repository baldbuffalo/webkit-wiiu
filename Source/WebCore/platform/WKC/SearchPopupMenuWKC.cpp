#include "config.h"
#include "SearchPopupMenuWKC.h"
#include "PopupMenuWKC.h"

namespace WebCore {

SearchPopupMenuWKC::SearchPopupMenuWKC(PopupMenuClient* client)
    : m_popup(adoptRef(*new PopupMenuWKC(client)))
{
}

PopupMenu* SearchPopupMenuWKC::popupMenu()
{
    return m_popup.ptr();
}

void SearchPopupMenuWKC::saveRecentSearches(const AtomString&, const Vector<RecentSearch>&)
{
}

void SearchPopupMenuWKC::loadRecentSearches(const AtomString&, Vector<RecentSearch>&)
{
}

bool SearchPopupMenuWKC::enabled()
{
    return false;
}

} // namespace WebCore
