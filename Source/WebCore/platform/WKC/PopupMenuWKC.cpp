#include "config.h"
#include "PopupMenuWKC.h"

#include "LocalFrameView.h"
#include "HostWindow.h"
#include "PopupMenuClient.h"
#include "NotImplemented.h"

#include "WKCWebViewPrivate.h"
#include "DropDownListClientWKC.h"
#include "helpers/privates/WKCPopupMenuClientPrivate.h"

namespace WebCore {

PopupMenuWKC::PopupMenuWKC(PopupMenuClient* client)
    : m_popupClient(client)
    , m_wkc(nullptr)
    , m_visible(false)
{
    if (client)
        m_wkc = new WKC::PopupMenuClientPrivate(client);
}

PopupMenuWKC::~PopupMenuWKC()
{
    hide();
    delete m_wkc;
}

void PopupMenuWKC::show(const IntRect& rect, LocalFrameView* view, int index)
{
    if (!client())
        return;

    if (m_visible)
        hide();

    auto* hostWindow = view ? view->hostWindow() : nullptr;
    if (!hostWindow)
        return;

    auto* pageclient = hostWindow->platformPageClient();
    if (pageclient) {
        WKC::WKCWebViewPrivate* webview = static_cast<WKC::WKCWebViewPrivate*>(pageclient);
        m_visible = true;
        webview->dropdownlistclient()->show(rect, reinterpret_cast<WebCore::FrameView*>(view), index, &m_wkc->wkc());
    }
}

void PopupMenuWKC::hide()
{
    if (!m_visible)
        return;

    m_visible = false;

    if (!m_wkc)
        return;

    notImplemented();
}

void PopupMenuWKC::updateFromElement()
{
    notImplemented();
}

void PopupMenuWKC::disconnectClient()
{
    m_popupClient = nullptr;
}

} // namespace WebCore
