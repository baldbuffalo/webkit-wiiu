/*
 *  Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
 *  Copyright (C) 2007, 2008, 2009 Holger Hans Peter Freyther
 *  Copyright (C) 2007 Christian Dywan <christian@twotoasts.de>
 *  Copyright (C) 2008, 2009 Collabora Ltd.  All rights reserved.
 *  Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 *  Copyright (C) Research In Motion Limited 2009. All rights reserved.
 *  Copyright (c) 2010-2017 ACCESS CO., LTD. All rights reserved.
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

#include "FrameLoaderClientWKC.h"
#include "Frame.h"
#include "LocalFrame.h"
#include "DocumentPage.h"
#include "DocumentView.h"
#include "FrameInlines.h"
#include "ReferrerPolicy.h"
#include "FrameIdentifier.h"
#include "FrameTreeSyncData.h"
#include "FrameLoader.h"
#include "EventHandler.h"
#include "FrameView.h"
#include "LocalFrameView.h"
#include "DocumentLoader.h"
#include "DocumentWriter.h"
#include "SharedBuffer.h"
#include "MIMETypeRegistry.h"
#include "UserAgent.h"
#include "HTMLFormElement.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "HTMLPlugInElement.h"
#include "Widget.h"
#include "Page.h"
#include "DOMWrapperWorld.h"
#include <wtf/UniqueRef.h>
#include <wtf/text/WTFString.h>
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "ResourceError.h"

#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"


#include "helpers/WKCFrame.h"

#ifdef WKC_ENABLE_CUSTOMJS
#include "helpers/WKCSettings.h"
#include <APICast.h>
#include <JSStringRef.h>
#include <JSObjectRef.h>
#include "CustomJS/privates/CustomJSCallBackWKC.h"
#endif // WKC_ENABLE_CUSTOMJS

#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCHTMLFormElementPrivate.h"
#include "helpers/privates/WKCHTMLFrameOwnerElementPrivate.h"
#include "helpers/privates/WKCPagePrivate.h"


// implementations

namespace WKC {

FrameLoaderClientWKC::FrameLoaderClientWKC(WebCore::FrameLoader& loader, WKCWebFramePrivate* frame)
     : WebCore::LocalFrameLoaderClient(loader),
       m_frame(frame)
{
}
FrameLoaderClientWKC::~FrameLoaderClientWKC()
{
    if (!m_frame)
        return;

    WKCWebFrame* frameWrapper = m_frame->m_parent;
    bool isMainFrame = m_frame->m_view && m_frame->m_view->m_mainFrame == frameWrapper;
    m_frame->coreFrameDestroyed();

    // A child WKCWebFrame is owned by its LocalFrame's loader client. The main
    // wrapper is retained by WKCWebViewPrivate until Page teardown completes.
    if (frameWrapper && !isMainFrame)
        WKCWebFrame::deleteWKCWebFrame(frameWrapper);
}

void
FrameLoaderClientWKC::updateFrameURI()
{
    if (!m_frame)
        return;
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = nullptr;
    }
    auto* frame = m_frame->core();
    if (!frame)
        return;
    if (auto* loader = frame->loader().activeDocumentLoader()) {
        auto uri = loader->url().string().utf8();
        m_frame->m_uri = fastStrDup(uri.data());
    }
}

void
FrameLoaderClientWKC::notifyStatus(WKC::LoadStatus loadStatus)
{
    if (!m_frame)
        return;

    m_frame->m_loadStatus = loadStatus;
    WKCWebViewPrivate* webView = m_frame->m_view;
    if (webView && m_frame->m_parent == webView->m_mainFrame)
        webView->m_loadStatus = loadStatus;
}

WKCWebFrame*
FrameLoaderClientWKC::webFrame() const
{
    return m_frame ? m_frame->parent() : nullptr;
}

RefPtr<WebCore::HistoryItem>
FrameLoaderClientWKC::createHistoryItemTree(bool, WebCore::BackForwardItemIdentifier) const
{
    // History-item tree construction is only needed for the multi-process
    // back/forward cache; the single-process WKC embedder has nothing to build.
    return nullptr;
}

bool
FrameLoaderClientWKC::hasWebView() const
{
    return true;
}

// callbacks

void
FrameLoaderClientWKC::makeRepresentation(WebCore::DocumentLoader* loader)
{
UNUSED_PARAM(loader);
}

void
FrameLoaderClientWKC::forceLayoutForNonHTML()
{

}


void
FrameLoaderClientWKC::setCopiesOnScroll()
{

}


void
FrameLoaderClientWKC::detachedFromParent2()
{
    if (!m_frame || !m_frame->core())
        return;
    WebCore::FrameLoader& frameLoader = m_frame->core()->loader();
    WebCore::DocumentLoader* documentLoader = frameLoader.activeDocumentLoader();
    if (!documentLoader)
        return;
    if (frameLoader.state() == WebCore::FrameState::CommittedPage)
        documentLoader->cancelMainResourceLoad(WebCore::FrameLoader::cancelledError(documentLoader->request()));
}

void
FrameLoaderClientWKC::detachedFromParent3()
{
    if (m_frame && m_frame->core())
        m_frame->core()->eventHandler().stopAutoscrollTimer();
}


void
FrameLoaderClientWKC::assignIdentifierToInitialRequest(WebCore::ResourceLoaderIdentifier identifier, WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& resource)
{
UNUSED_PARAM(identifier); UNUSED_PARAM(loader); UNUSED_PARAM(resource);
}


void
FrameLoaderClientWKC::dispatchWillSendRequest(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, WebCore::ResourceRequest& request, const WebCore::ResourceResponse& redirect_response)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); UNUSED_PARAM(request); UNUSED_PARAM(redirect_response);
}

bool
FrameLoaderClientWKC::shouldUseCredentialStorage(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); return false;
}

void
FrameLoaderClientWKC::dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::AuthenticationChallenge& challenge)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); UNUSED_PARAM(challenge);
}

void
FrameLoaderClientWKC::dispatchDidReceiveResponse(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceResponse& response)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); UNUSED_PARAM(response);
}

void
FrameLoaderClientWKC::dispatchDidReceiveContentLength(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, int lengthReceived)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); UNUSED_PARAM(lengthReceived);
}

void
FrameLoaderClientWKC::dispatchDidFinishLoading(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier);
}

void
FrameLoaderClientWKC::dispatchDidFailLoading(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceError& error)
{
UNUSED_PARAM(loader); UNUSED_PARAM(identifier); UNUSED_PARAM(error);
}

bool
FrameLoaderClientWKC::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse& response, int length)
{
UNUSED_PARAM(loader); UNUSED_PARAM(request); UNUSED_PARAM(response); UNUSED_PARAM(length); return false;
}

void
FrameLoaderClientWKC::dispatchDidDispatchOnloadEvents()
{

}

void
FrameLoaderClientWKC::dispatchDidReceiveServerRedirectForProvisionalLoad()
{

}

void
FrameLoaderClientWKC::dispatchDidCancelClientRedirect()
{

}

void
FrameLoaderClientWKC::dispatchWillPerformClientRedirect(const WTF::URL& uri, double interval, WallTime fireDate, WebCore::LockBackForwardList)
{
UNUSED_PARAM(uri); UNUSED_PARAM(interval); UNUSED_PARAM(fireDate);
}

void
FrameLoaderClientWKC::dispatchDidChangeLocationWithinPage()
{
    updateFrameURI();
}

void
FrameLoaderClientWKC::dispatchDidNavigateWithinPage()
{
    updateFrameURI();
}

void
FrameLoaderClientWKC::dispatchDidPushStateWithinPage()
{
    updateFrameURI();
}

void
FrameLoaderClientWKC::dispatchDidReplaceStateWithinPage()
{
    updateFrameURI();
}

void
FrameLoaderClientWKC::dispatchDidPopStateWithinPage()
{
    updateFrameURI();
}

void
FrameLoaderClientWKC::dispatchWillClose()
{

}

void
FrameLoaderClientWKC::dispatchDidReceiveIcon()
{

}

void
FrameLoaderClientWKC::dispatchDidStartProvisionalLoad()
{
    updateFrameURI();
    notifyStatus(WKC::ELoadStatusProvisional);
}

void
FrameLoaderClientWKC::dispatchDidReceiveTitle(const WebCore::StringWithDirection& title)
{
    if (!m_frame)
        return;
    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = nullptr;
    }
    const auto& value = title.string;
    unsigned length = value.length();
    auto* buffer = static_cast<unsigned short*>(fastMalloc((length + 1) * sizeof(unsigned short)));
    if (!buffer)
        return;
    for (unsigned i = 0; i < length; ++i)
        buffer[i] = value[i];
    buffer[length] = 0;
    m_frame->m_title = buffer;
}

void
FrameLoaderClientWKC::dispatchDidCommitLoad(const std::optional<WebCore::BackForwardCacheCommitData>&)
{
    if (!m_frame)
        return;
    updateFrameURI();
    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = nullptr;
    }
    notifyStatus(WKC::ELoadStatusCommitted);
}

void
FrameLoaderClientWKC::dispatchDidFailProvisionalLoad(const WebCore::ResourceError& error, WebCore::WillContinueLoading willContinueLoading, WebCore::WillInternallyHandleFailure willInternallyHandleFailure)
{
    UNUSED_PARAM(error);

    // A provisional failure is not final when WebCore will recover or continue
    // the navigation (for example, by retrying with an HTTP fallback).
    if (willContinueLoading == WebCore::WillContinueLoading::Yes
        || willInternallyHandleFailure == WebCore::WillInternallyHandleFailure::Yes) {
        notifyStatus(WKC::ELoadStatusNone);
        return;
    }

    notifyStatus(WKC::ELoadStatusFailed);
}

void
FrameLoaderClientWKC::dispatchDidFailLoad(const WebCore::ResourceError& error)
{
    UNUSED_PARAM(error);
    notifyStatus(WKC::ELoadStatusFailed);
}

void
FrameLoaderClientWKC::dispatchDidFinishDocumentLoad()
{

}

void
FrameLoaderClientWKC::dispatchDidFinishLoad()
{
    notifyStatus(WKC::ELoadStatusFinished);
}

void
FrameLoaderClientWKC::dispatchDidLayout()
{
    // Report the first visual update once the committed document is laid out.
    // notifyStatus changes the state, so subsequent layout passes do not
    // repeatedly emit the first-visual state or overwrite a finished load.
    if (m_frame && m_frame->m_loadStatus == WKC::ELoadStatusCommitted)
        notifyStatus(WKC::ELoadStatusFirstVisual);
}

WebCore::LocalFrame*
FrameLoaderClientWKC::dispatchCreatePage(const WebCore::NavigationAction& action, WebCore::NewFrameOpenerPolicy openerPolicy, const WTF::String& openedMainFrameName)
{
    UNUSED_PARAM(action);
    UNUSED_PARAM(openerPolicy);
    UNUSED_PARAM(openedMainFrameName);
    return nullptr;
}

void
FrameLoaderClientWKC::dispatchShow()
{

}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNewWindowAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, WebCore::FormState* state, const WTF::String& frame_name, std::optional<WebCore::HitTestResult>&&, WebCore::FramePolicyFunction&& function)
{
    UNUSED_PARAM(action);
    UNUSED_PARAM(request);
    UNUSED_PARAM(state);
    UNUSED_PARAM(frame_name);
    function(WebCore::PolicyAction::Ignore);
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNavigationAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse&, WebCore::FormState* state, const WTF::String&, std::optional<WebCore::NavigationIdentifier>, std::optional<WebCore::HitTestResult>&&, bool, WebCore::NavigationUpgradeToHTTPSBehavior, WebCore::SandboxFlags, WebCore::PolicyDecisionMode, WebCore::FramePolicyFunction&& function)
{
    UNUSED_PARAM(action);
    UNUSED_PARAM(request);
    function(WebCore::PolicyAction::Use);
}

void
FrameLoaderClientWKC::cancelPolicyCheck()
{

}


void
FrameLoaderClientWKC::dispatchUnableToImplementPolicy(const WebCore::ResourceError& error)
{
UNUSED_PARAM(error);
}


void
FrameLoaderClientWKC::dispatchWillSubmitForm(WebCore::FormState& state, WTF::URL&& requestURL, WTF::String&& method, CompletionHandler<void()>&& handler)
{
    UNUSED_PARAM(state);
    UNUSED_PARAM(requestURL);
    UNUSED_PARAM(method);
    handler();
}

void
FrameLoaderClientWKC::revertToProvisionalState(WebCore::DocumentLoader* loader)
{
UNUSED_PARAM(loader);
}

void
FrameLoaderClientWKC::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
{
UNUSED_PARAM(loader); UNUSED_PARAM(error);
}

RefPtr<WebCore::LocalFrame>
FrameLoaderClientWKC::createFrame(const AtomString& name, WebCore::HTMLFrameOwnerElement& ownerElement)
{
    WebCore::LocalFrame* parentFrame = m_frame ? m_frame->core() : nullptr;
    if (!parentFrame || !m_frame->m_view)
        return nullptr;
    WebCore::Page* page = parentFrame->page();
    if (!page)
        return nullptr;

    // Create the WKC wrapper for the child frame; its core LocalFrame is created
    // by LocalFrame::createSubframe below, which also builds this frame's
    // FrameLoaderClientWKC through the ClientCreator lambda.
    HTMLFrameOwnerElementPrivate ownerPrivate(&ownerElement);
    WKC::WKCWebFrame* child = WKC::WKCWebFrame::create(m_frame->m_view, reinterpret_cast<WKC::HTMLFrameOwnerElement*>(&ownerPrivate.wkc()));
    if (!child)
        return nullptr;
    WKCWebFramePrivate* childPrivate = child->privateFrame();

    auto effectiveSandboxFlags = ownerElement.sandboxFlags();
    Ref<WebCore::LocalFrame> subframe = WebCore::LocalFrame::createSubframe(*page,
        [childPrivate](auto&, auto& frameLoader) {
            return makeUniqueRefWithoutRefCountedCheck<FrameLoaderClientWKC>(frameLoader, childPrivate);
        },
        WebCore::generateFrameIdentifier(), effectiveSandboxFlags, WebCore::ReferrerPolicy::EmptyString,
        ownerElement, WebCore::FrameTreeSyncData::create());

    childPrivate->setCoreFrame(subframe.ptr());
    subframe->init();

    // The creation may have run JavaScript that removed the frame from the page.
    if (!subframe->page())
        return nullptr;

    return subframe.ptr();
}

AtomString
FrameLoaderClientWKC::overrideMediaType() const
{
    return nullAtom();
}

#ifdef WKC_ENABLE_CUSTOMJS
static void
_setCustomJS(CustomJSAPIListHashMap *hashMap, JSGlobalContextRef ctx, JSObjectRef windowObject, bool isStringApi)
{
    CustomJSAPIListHashMap::iterator end;
    JSStringRef jsFuncName;
    JSObjectRef jsFunc;

    if (!hashMap || !hashMap->size())
        return;

    end = hashMap->end();
    for (CustomJSAPIListHashMap::iterator iter = hashMap->begin(); iter != end; ++iter) {
        jsFuncName = JSStringCreateWithUTF8CString(iter->first.utf8().data());
        if (isStringApi) {
            jsFunc = JSObjectMakeFunctionWithCallback(ctx, jsFuncName, (JSObjectCallAsFunctionCallback)CustomJSStringCallBackWKC);
        } else {
            jsFunc = JSObjectMakeFunctionWithCallback(ctx, jsFuncName, (JSObjectCallAsFunctionCallback)CustomJSCallBackWKC);
        }
        JSObjectSetProperty(ctx, windowObject, jsFuncName, jsFunc, kJSPropertyAttributeNone, NULL);
        JSStringRelease(jsFuncName);
    }
}
#endif // WKC_ENABLE_CUSTOMJS

void
FrameLoaderClientWKC::dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld& world)
{
    if (!m_frame || &world != &WebCore::mainThreadNormalWorldSingleton())
        return;
#ifdef WKC_ENABLE_CUSTOMJS
    WKCSettings* settings = m_frame->m_view ? m_frame->m_view->settings() : nullptr;
    bool isCustomJSEnable = settings && settings->isScriptEnabled();
    if (isCustomJSEnable)
        m_frame->initCustomJSAPIList();
    if (!isCustomJSEnable)
        return;
    WebCore::LocalFrame* coreFrame = m_frame->core();
    if (!coreFrame)
        return;
    JSGlobalContextRef context = toGlobalRef(coreFrame->script().globalObject(world)->globalExec());
    JSObjectRef windowObject = toRef(coreFrame->script().globalObject(world));
    if (!windowObject)
        return;
    _setCustomJS(m_frame->getCustomJSAPIList(), context, windowObject, false);
    _setCustomJS(m_frame->getCustomJSAPIListInternal(), context, windowObject, false);
    _setCustomJS(m_frame->getCustomJSStringAPIList(), context, windowObject, true);
    _setCustomJS(m_frame->getCustomJSStringAPIListInternal(), context, windowObject, true);
#endif // WKC_ENABLE_CUSTOMJS
}

void
FrameLoaderClientWKC::setMainFrameDocumentReady(bool flag)
{
UNUSED_PARAM(flag);
}


void
FrameLoaderClientWKC::startDownload(const WebCore::ResourceRequest& request, const WTF::String& suggestedName, WebCore::FromDownloadAttribute)
{
UNUSED_PARAM(request); UNUSED_PARAM(suggestedName);
}


void
FrameLoaderClientWKC::willChangeTitle(WebCore::DocumentLoader* loader)
{
UNUSED_PARAM(loader);
}

void
FrameLoaderClientWKC::didChangeTitle(WebCore::DocumentLoader* loader)
{
UNUSED_PARAM(loader);
}


void
FrameLoaderClientWKC::committedLoad(WebCore::DocumentLoader* loader, const WebCore::SharedBuffer& data)
{
    if (!loader)
        return;
    WebCore::DocumentWriter& writer = loader->writer();
    const WTF::String& encoding = loader->overrideEncoding();
    if (encoding.isNull())
        writer.setEncoding(loader->response().textEncodingName(), WebCore::DocumentWriter::IsEncodingUserChosen::No);
    else
        writer.setEncoding(encoding, WebCore::DocumentWriter::IsEncodingUserChosen::Yes);
    loader->commitData(data);
}

void
FrameLoaderClientWKC::finishedLoading(WebCore::DocumentLoader* loader)
{
UNUSED_PARAM(loader);
}


void
FrameLoaderClientWKC::updateGlobalHistory()
{

}

void
FrameLoaderClientWKC::updateGlobalHistoryRedirectLinks()
{

}

WebCore::ShouldGoToHistoryItem
FrameLoaderClientWKC::shouldGoToHistoryItem(WebCore::HistoryItem& item, WebCore::IsSameDocumentNavigation) const
{
UNUSED_PARAM(item); return WebCore::ShouldGoToHistoryItem::Yes;
}


bool
FrameLoaderClientWKC::shouldFallBack(const WebCore::ResourceError& error) const
{
UNUSED_PARAM(error); return false;
}


bool
FrameLoaderClientWKC::canHandleRequest(const WebCore::ResourceRequest& request) const
{
UNUSED_PARAM(request); return true;
}

bool
FrameLoaderClientWKC::canShowMIMEType(const WTF::String& type) const
{
return WebCore::MIMETypeRegistry::canShowMIMEType(type);
}

bool
FrameLoaderClientWKC::representationExistsForURLScheme(WTF::StringView string) const
{
UNUSED_PARAM(string); return false;
}

WTF::String
FrameLoaderClientWKC::generatedMIMETypeForURLScheme(WTF::StringView string) const
{
UNUSED_PARAM(string); return { };
}


void
FrameLoaderClientWKC::frameLoadCompleted()
{

}

void
FrameLoaderClientWKC::saveViewStateToItem(WebCore::HistoryItem& item)
{
UNUSED_PARAM(item);
}

void
FrameLoaderClientWKC::restoreViewState()
{

}

void
FrameLoaderClientWKC::provisionalLoadStarted()
{

}

void
FrameLoaderClientWKC::didFinishLoad()
{

}

void
FrameLoaderClientWKC::prepareForDataSourceReplacement()
{

}


Ref<WebCore::DocumentLoader>
FrameLoaderClientWKC::createDocumentLoader(WebCore::ResourceRequest&& request, WebCore::SubstituteData&& substituteData)
{
    return WebCore::DocumentLoader::create(WTF::move(request), WTF::move(substituteData));
}

Ref<WebCore::DocumentLoader>
FrameLoaderClientWKC::createDocumentLoader(WebCore::ResourceRequest&& request, WebCore::SubstituteData&& substituteData, WebCore::ResourceRequest&&)
{
    // The overrideRequest is used by the multi-process loader path; the
    // single-process WKC embedder ignores it and builds the loader as usual.
    return WebCore::DocumentLoader::create(WTF::move(request), WTF::move(substituteData));
}

void
FrameLoaderClientWKC::setTitle(const WebCore::StringWithDirection& title, const WTF::URL& uri)
{
UNUSED_PARAM(title); UNUSED_PARAM(uri);
}


WTF::String
FrameLoaderClientWKC::userAgent(const WTF::URL& uri) const
{
return WebCore::standardUserAgentForURL(uri);
}


void
FrameLoaderClientWKC::savePlatformDataToCachedFrame(WebCore::CachedFrame* frame)
{
UNUSED_PARAM(frame);
}

void
FrameLoaderClientWKC::transitionToCommittedFromCachedFrame(WebCore::CachedFrame* frame)
{
UNUSED_PARAM(frame);
}

void
FrameLoaderClientWKC::transitionToCommittedForNewPage(InitializingIframe)
{
    WebCore::LocalFrame* frame = m_frame ? m_frame->core() : nullptr;
    WKCWebViewPrivate* containingWindow = m_frame ? m_frame->m_view : nullptr;
    if (!frame || !containingWindow)
        return;
#if ENABLE(WKC_ANDROID_LAYOUT)
    int screenWidth = frame->view() ? frame->view()->screenWidth() : 0;
#endif
    WebCore::IntSize desktopSize = containingWindow->defaultDesktopSize();
    WebCore::IntSize viewSize = containingWindow->defaultViewSize();
    bool transparent = containingWindow->transparent();
    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparentBlack : WebCore::Color::white;
    bool useFixedLayout = frame->view() && frame->view()->useFixedLayout();
    WebCore::ScrollbarMode scrollbarMode = frame->ownerElement() ? WebCore::ScrollbarMode::Auto : WebCore::ScrollbarMode::AlwaysOff;
    frame->createView(desktopSize, backgroundColor, viewSize, useFixedLayout, scrollbarMode, false, scrollbarMode, false);
    if (frame->view() && !frame->ownerElement())
        frame->view()->setCanHaveScrollbars(false);
#if ENABLE(WKC_ANDROID_LAYOUT)
    if (frame->view())
        frame->view()->setScreenWidth(screenWidth);
#endif
}


bool
FrameLoaderClientWKC::canCachePage() const
{
return false;
}

void
FrameLoaderClientWKC::convertMainResourceLoadToDownload(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
{

}

RefPtr<WebCore::Widget>
FrameLoaderClientWKC::createPlugin(WebCore::HTMLPlugInElement&, const WTF::URL&, const Vector<AtomString>&, const Vector<AtomString>&, const WTF::String&, bool)
{
    // NPAPI/Netscape plugins are not supported on Wii U.
    return nullptr;
}

void
FrameLoaderClientWKC::redirectDataToPlugin(WebCore::Widget& widget)
{
    UNUSED_PARAM(widget);
}

void
FrameLoaderClientWKC::sendH2Ping(const WTF::URL& url, CompletionHandler<void(std::expected<Seconds, WebCore::ResourceError>&&)>&& completionHandler)
{
    completionHandler(std::unexpected(WebCore::internalError(url)));
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForResponse(const WebCore::ResourceResponse& response, const WebCore::ResourceRequest& request, const WTF::String&, WebCore::FramePolicyFunction&& function)
{
    UNUSED_PARAM(response);
    UNUSED_PARAM(request);
    function(WebCore::PolicyAction::Use);
}

void
FrameLoaderClientWKC::dispatchWillSendSubmitEvent(Ref<WebCore::FormState>&& state)
{
UNUSED_PARAM(state);
}

bool
FrameLoaderClientWKC::canShowMIMETypeAsHTML(const WTF::String& MIMEType) const
{
    return MIMEType == "text/html"_s || MIMEType == "application/xhtml+xml"_s;
}

WebCore::ObjectContentType
FrameLoaderClientWKC::objectContentType(const WTF::URL& url, const WTF::String& mime)
{
    UNUSED_PARAM(url);
    return WebCore::MIMETypeRegistry::isSupportedImageMIMEType(mime)
        ? WebCore::ObjectContentType::Image
        : WebCore::ObjectContentType::None;
}

Ref<WebCore::FrameNetworkingContext>
FrameLoaderClientWKC::createNetworkingContext()
{
    WKC::FrameNetworkingContextWKC* ctx = WKC::FrameNetworkingContextWKC::create(m_frame ? m_frame->core() : 0);
    return adoptRef(*ctx);
}

FrameNetworkingContextWKC::FrameNetworkingContextWKC(WebCore::LocalFrame* frame)
    : FrameNetworkingContext(frame)
{
}

FrameNetworkingContextWKC*
FrameNetworkingContextWKC::create(WebCore::LocalFrame* frame)
{
    FrameNetworkingContextWKC* self = 0;
    self = new FrameNetworkingContextWKC(frame);
    return self;
}

WebCore::FrameLoaderClient*
FrameNetworkingContextWKC::frameLoaderClient() const
{
    if (frame())
        return &frame()->loader().client();
    return 0;
}

void
FrameLoaderClientWKC::didChangeScrollOffset()
{

}

bool
FrameLoaderClientWKC::allowScript(bool enabledPerSettings)
{
return enabledPerSettings;
}

bool
FrameLoaderClientWKC::shouldForceUniversalAccessFromLocalURL(const WTF::URL& url)
{
UNUSED_PARAM(url); return false;
}

void
FrameLoaderClientWKC::dispatchGlobalObjectAvailable(WebCore::DOMWrapperWorld& world)
{
UNUSED_PARAM(world);
}

} // namespace
