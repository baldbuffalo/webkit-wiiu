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

#include "CachedFrame.h"
#include "FrameLoaderClientWKC.h"
#include "Frame.h"
#include "LocalFrame.h"
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
#include "FormState.h"
#include "HistoryItem.h"
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
#include "ResourceHandle.h"
#include "HitTestResult.h"

#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"

#include "helpers/FrameLoaderClientIf.h"

#include "helpers/WKCFrame.h"
#include "helpers/WKCURL.h"
#include "helpers/WKCString.h"

#ifdef WKC_ENABLE_CUSTOMJS
#include "helpers/WKCSettings.h"
#include <APICast.h>
#include <JSStringRef.h>
#include <JSObjectRef.h>
#include "CustomJS/privates/CustomJSCallBackWKC.h"
#endif // WKC_ENABLE_CUSTOMJS

#include "helpers/privates/WKCAuthenticationChallengePrivate.h"
#include "helpers/privates/WKCCachedFramePrivate.h"
#include "helpers/privates/WKCDOMWrapperWorldPrivate.h"
#include "helpers/privates/WKCDocumentLoaderPrivate.h"
#include "helpers/privates/WKCFormStatePrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCHTMLFormElementPrivate.h"
#include "helpers/privates/WKCHTMLFrameOwnerElementPrivate.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"
#include "helpers/privates/WKCPagePrivate.h"
#include "helpers/privates/WKCResourceErrorPrivate.h"
#include "helpers/privates/WKCResourceLoaderPrivate.h"
#include "helpers/privates/WKCNavigationActionPrivate.h"
#include "helpers/privates/WKCResourceRequestPrivate.h"
#include "helpers/privates/WKCResourceResponsePrivate.h"
#include "helpers/privates/WKCSecurityOriginPrivate.h"

#include "NotImplemented.h"

// implementations

namespace WKC {

FrameLoaderClientWKC::FrameLoaderClientWKC(WebCore::FrameLoader& loader, WKCWebFramePrivate* frame)
     : WebCore::LocalFrameLoaderClient(loader),
       m_frame(frame),
       m_appClient(0)
{
    m_policyDecision = 0;
    if (m_frame)
        construct();
}
FrameLoaderClientWKC::~FrameLoaderClientWKC()
{
    if (m_appClient) {
        if (m_frame) {
            m_frame->clientBuilders().deleteFrameLoaderClient(m_appClient);
        }
        m_appClient = 0;
    }
}

bool
FrameLoaderClientWKC::construct()
{
    m_appClient = m_frame->clientBuilders().createFrameLoaderClient(m_frame->parent());
    if (!m_appClient) return false;
    return true;
}

void
FrameLoaderClientWKC::notifyStatus(WKC::LoadStatus loadStatus)
{
    m_frame->m_loadStatus = loadStatus;
    WKCWebViewPrivate* webView = m_frame->m_view;
    if (m_frame->m_parent == webView->m_mainFrame) {
        webView->m_loadStatus = loadStatus;
    }
}

WKCWebFrame*
FrameLoaderClientWKC::webFrame() const
{
    return m_frame->parent();
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
    DocumentLoaderPrivate ldr(loader);
    m_appClient->makeRepresentation(&ldr.wkc());
}

void
FrameLoaderClientWKC::forceLayoutForNonHTML()
{
    m_appClient->forceLayoutForNonHTML();
}


void
FrameLoaderClientWKC::setCopiesOnScroll()
{
    m_appClient->setCopiesOnScroll();
}


void
FrameLoaderClientWKC::detachedFromParent2()
{
    m_appClient->detachedFromParent2();

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
    m_appClient->detachedFromParent3();
    // If we are pan-scrolling when frame is detached, stop the pan scrolling.
    m_frame->core()->eventHandler().stopAutoscrollTimer();
}


void
FrameLoaderClientWKC::assignIdentifierToInitialRequest(WebCore::ResourceLoaderIdentifier identifier, WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& resource)
{
    ResourceRequestPrivate req(resource);
    DocumentLoaderPrivate ldr(loader);
    m_appClient->assignIdentifierToInitialRequest(identifier.toUInt64(), &ldr.wkc(), req.wkc());
}


void
FrameLoaderClientWKC::dispatchWillSendRequest(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, WebCore::ResourceRequest& request, const WebCore::ResourceResponse& redirect_response)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceRequestPrivate req(request);
    ResourceResponsePrivate res(redirect_response);
    m_appClient->dispatchWillSendRequest(&ldr.wkc(), identifier.toUInt64(), req.wkc(), res.wkc());
}

bool
FrameLoaderClientWKC::shouldUseCredentialStorage(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier)
{
    bool ret = false;
    DocumentLoaderPrivate ldr(loader);
    ret = m_appClient->shouldUseCredentialStorage(&ldr.wkc(), identifier.toUInt64());
    return ret;
}

void
FrameLoaderClientWKC::dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::AuthenticationChallenge& challenge)
{
    DocumentLoaderPrivate ldr(loader);
    AuthenticationChallengePrivate wc(challenge);
    m_appClient->dispatchDidReceiveAuthenticationChallenge(&ldr.wkc(), identifier.toUInt64(), wc.wkc());
}

void
FrameLoaderClientWKC::dispatchDidReceiveResponse(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceResponse& response)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceResponsePrivate res(response);
    m_appClient->dispatchDidReceiveResponse(&ldr.wkc(), identifier.toUInt64(), res.wkc());
}

void
FrameLoaderClientWKC::dispatchDidReceiveContentLength(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, int lengthReceived)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->dispatchDidReceiveContentLength(&ldr.wkc(), identifier.toUInt64(), lengthReceived);
}

void
FrameLoaderClientWKC::dispatchDidFinishLoading(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->dispatchDidFinishLoading(&ldr.wkc(), identifier.toUInt64());
}

void
FrameLoaderClientWKC::dispatchDidFailLoading(WebCore::DocumentLoader* loader, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceError& error)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailLoading(&ldr.wkc(), identifier.toUInt64(), wobj.wkc());
}

bool
FrameLoaderClientWKC::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse& response, int length)
{
    bool ret = false;
    DocumentLoaderPrivate ldr(loader);
    ResourceRequestPrivate req(request);
    ResourceResponsePrivate res(response);
    ret = m_appClient->dispatchDidLoadResourceFromMemoryCache(&ldr.wkc(), req.wkc(), res.wkc(), length);
    return ret;
}

void
FrameLoaderClientWKC::dispatchDidDispatchOnloadEvents()
{
    m_appClient->dispatchDidHandleOnloadEvents();
}

void
FrameLoaderClientWKC::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    m_appClient->dispatchDidReceiveServerRedirectForProvisionalLoad();
}

void
FrameLoaderClientWKC::dispatchDidCancelClientRedirect()
{
    m_appClient->dispatchDidCancelClientRedirect();
}

void
FrameLoaderClientWKC::dispatchWillPerformClientRedirect(const WTF::URL& uri, double interval, WallTime fireDate, WebCore::LockBackForwardList)
{
    m_appClient->dispatchWillPerformClientRedirect(uri, interval, fireDate.secondsSinceEpoch().value());
}

void
FrameLoaderClientWKC::dispatchDidChangeLocationWithinPage()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidChangeLocationWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidNavigateWithinPage()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidNavigateWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidPushStateWithinPage()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidPushStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidReplaceStateWithinPage()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidReplaceStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidPopStateWithinPage()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidPopStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchWillClose()
{
    m_appClient->dispatchWillClose();
}

void
FrameLoaderClientWKC::dispatchDidReceiveIcon()
{
    m_appClient->dispatchDidReceiveIcon();
}

void
FrameLoaderClientWKC::dispatchDidStartProvisionalLoad()
{
    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    m_appClient->dispatchDidStartProvisionalLoad();
    notifyStatus(WKC::ELoadStatusProvisional);
}

void
FrameLoaderClientWKC::dispatchDidReceiveTitle(const WebCore::StringWithDirection& title)
{
    WKC::String str(title.string);
    str.setDirection(title.direction==WebCore::TextDirection::LTR ? WKC::LTR : WKC::RTL);
    m_appClient->dispatchDidReceiveTitle(str);

    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = 0;
    }
    {
        auto titleString = title.string;
        unsigned len = titleString.length();
        unsigned short* buf = static_cast<unsigned short*>(fastMalloc((len + 1) * sizeof(unsigned short)));
        for (unsigned i = 0; i < len; ++i)
            buf[i] = titleString[i];
        buf[len] = 0;
        m_frame->m_title = buf;
    }
    // just ignore the error
}

void
FrameLoaderClientWKC::dispatchDidCommitLoad(std::optional<WebCore::HasInsecureContent>, std::optional<WebCore::UsedLegacyTLS>, std::optional<WebCore::WasPrivateRelayed>)
{
    m_appClient->dispatchDidCommitLoad();

    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    if (m_frame->core()->loader().activeDocumentLoader())
        m_frame->m_uri = fastStrDup(m_frame->core()->loader().activeDocumentLoader()->url().string().utf8().data());

    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = 0;
    }

    notifyStatus(WKC::ELoadStatusCommitted);
}

void
FrameLoaderClientWKC::dispatchDidFailProvisionalLoad(const WebCore::ResourceError& error, WebCore::WillContinueLoading, WebCore::WillInternallyHandleFailure)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailProvisionalLoad(wobj.wkc());
    notifyStatus(WKC::ELoadStatusNone);
}

void
FrameLoaderClientWKC::dispatchDidFailLoad(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailLoad(wobj.wkc());
    notifyStatus(WKC::ELoadStatusFailed);
}

void
FrameLoaderClientWKC::dispatchDidFinishDocumentLoad()
{
    m_appClient->dispatchDidFinishDocumentLoad();
}

void
FrameLoaderClientWKC::dispatchDidFinishLoad()
{
    m_appClient->dispatchDidFinishLoad();
    notifyStatus(WKC::ELoadStatusFinished);
}

void
FrameLoaderClientWKC::dispatchDidLayout()
{
    m_appClient->dispatchDidLayout();
}

// WKC extension
bool FrameLoaderClientWKC::dispatchWillAcceptCookie(bool income, WebCore::ResourceHandle* handle, const WTF::String& url, const WTF::String& firstparty_host, const WTF::String& cookie_domain)
{
    if (handle) {
        ResourceHandlePrivate ldr(handle);
        return m_appClient->dispatchWillAcceptCookie(income, &ldr.wkc(), url.utf8().data(), firstparty_host.utf8().data(), cookie_domain.utf8().data());
    } else {
        return m_appClient->dispatchWillAcceptCookie(income, 0, url.utf8().data(), firstparty_host.utf8().data(), cookie_domain.utf8().data());
    }
}

bool
FrameLoaderClientWKC::dispatchWillReceiveData(WebCore::ResourceHandle* handle, int length)
{
    ResourceHandlePrivate ldr(handle);
    return m_appClient->dispatchWillReceiveData(&ldr.wkc(), length);
}

bool
FrameLoaderClientWKC::notifySSLHandshakeStatus(WebCore::ResourceHandle* handle, int status)
{
    ResourceHandlePrivate hdl(handle);
    return m_appClient->notifySSLHandshakeStatus(&hdl.wkc(), (WKC::SSLHandshakeStatus)status);
}

int
FrameLoaderClientWKC::dispatchWillPermitSendRequest(WebCore::ResourceHandle* handle, const char* url, int composition, bool isSync, const WebCore::ResourceResponse& redirect_response)
{
    ResourceResponsePrivate res(redirect_response);
    return m_appClient->dispatchWillPermitSendRequest((void*)handle, url, (WKC::ContentComposition)composition, isSync, res.wkc());
}

#ifdef WKC_ENABLE_CUSTOMJS
bool
FrameLoaderClientWKC::dispatchWillCallCustomJS(WKCCustomJSAPIList* api, void** context)
{
    return m_appClient->dispatchWillCallCustomJS(api, context);
}
#endif // WKC_ENABLE_CUSTOMJS

WebCore::LocalFrame*
FrameLoaderClientWKC::dispatchCreatePage(const WebCore::NavigationAction&, WebCore::NewFrameOpenerPolicy, const WTF::String&)
{
    WKC::Frame* ret = m_appClient->dispatchCreatePage();
    if (!ret)
        return 0;
    return (WebCore::LocalFrame *)ret->priv().webcore();
}

void
FrameLoaderClientWKC::dispatchShow()
{
    m_appClient->dispatchShow();
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNewWindowAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, WebCore::FormState* state, const WTF::String& frame_name, std::optional<WebCore::HitTestResult>&&, WebCore::FramePolicyFunction&& function)
{
    auto* heapFunc = new WebCore::FramePolicyFunction(WTFMove(function));
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)heapFunc);
    ResourceRequestPrivate req(request);
    NavigationActionPrivate nav(action);
    FormStatePrivate st(state);
    m_appClient->dispatchDecidePolicyForNewWindowAction(*f, nav.wkc(), req.wkc(), &st.wkc(), frame_name);
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNavigationAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse&, WebCore::FormState* state, const WTF::String&, std::optional<WebCore::NavigationIdentifier>, std::optional<WebCore::HitTestResult>&&, bool, WebCore::NavigationUpgradeToHTTPSBehavior, WebCore::SandboxFlags, WebCore::PolicyDecisionMode, WebCore::FramePolicyFunction&& function)
{
    auto* heapFunc = new WebCore::FramePolicyFunction(WTFMove(function));
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)heapFunc);
    ResourceRequestPrivate req(request);
    NavigationActionPrivate nav(action);
    FormStatePrivate st(state);
    m_appClient->dispatchDecidePolicyForNavigationAction(*f, nav.wkc(), req.wkc(), &st.wkc());
}

void
FrameLoaderClientWKC::cancelPolicyCheck()
{
    m_appClient->cancelPolicyCheck();
}


void
FrameLoaderClientWKC::dispatchUnableToImplementPolicy(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchUnableToImplementPolicy(wobj.wkc());
}


void
FrameLoaderClientWKC::dispatchWillSubmitForm(WebCore::FormState& state, WTF::URL&&, WTF::String&&, CompletionHandler<void()>&& handler)
{
    // Modern WebKit hands us a void completion handler (forms always submit);
    // wrap it so the WKC app policy callback, which still speaks in PolicyAction,
    // continues the submission when it replies.
    auto* heapFunc = new WebCore::FramePolicyFunction([completion = WTFMove(handler)](WebCore::PolicyAction) mutable { completion(); });
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)heapFunc);
    FormStatePrivate st(&state);
    m_appClient->dispatchWillSubmitForm(*f, &st.wkc());
}

void
FrameLoaderClientWKC::revertToProvisionalState(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->revertToProvisionalState(&ldr.wkc());
}

void
FrameLoaderClientWKC::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
{
    {
        DocumentLoaderPrivate ldr(loader);
        ResourceErrorPrivate wobj(error);
        m_appClient->setMainDocumentError(&ldr.wkc(), wobj.wkc());
    }
}

RefPtr<WebCore::LocalFrame>
FrameLoaderClientWKC::createFrame(const AtomString& name, WebCore::HTMLFrameOwnerElement& ownerElement)
{
    WebCore::LocalFrame* parentFrame = m_frame->core();
    if (!parentFrame)
        return nullptr;
    WebCore::Page* page = parentFrame->page();
    if (!page)
        return nullptr;

    // Create the WKC wrapper for the child frame; its core LocalFrame is created
    // by LocalFrame::createSubframe below, which also builds this frame's
    // FrameLoaderClientWKC through the ClientCreator lambda.
    HTMLFrameOwnerElementPrivate ownerPrivate(&ownerElement);
    WKC::WKCWebFrame* child = WKC::WKCWebFrame::create(m_frame->m_view, m_frame->clientBuilders(), reinterpret_cast<WKC::HTMLFrameOwnerElement*>(&ownerPrivate.wkc()));
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
    if (&world != &WebCore::mainThreadNormalWorldSingleton()) {
        return;
    }
    DOMWrapperWorldPrivate w(&world);

#ifdef WKC_ENABLE_CUSTOMJS
    WKCSettings* settings = m_frame->m_view->settings();
    bool isCustomJSEnable = (settings && settings->isScriptEnabled() && world);
    if (isCustomJSEnable) {
        m_frame->initCustomJSAPIList();
    }
#endif // WKC_ENABLE_CUSTOMJS

    m_appClient->dispatchDidClearWindowObjectInWorld(&w.wkc());

#ifdef WKC_ENABLE_CUSTOMJS
    if (isCustomJSEnable) {
        WebCore::LocalFrame* coreFrame = m_frame->core();
        if (!coreFrame)
            return;

        JSGlobalContextRef ctx = toGlobalRef(coreFrame->script().globalObject(world)->globalExec());
        JSObjectRef windowObject = toRef(coreFrame->script().globalObject(world));
        if (!windowObject)
            return;

        //integer external
        _setCustomJS(m_frame->getCustomJSAPIList(), ctx, windowObject, false);

        //integer internal
        _setCustomJS(m_frame->getCustomJSAPIListInternal(), ctx, windowObject, false);

        //string external
        _setCustomJS(m_frame->getCustomJSStringAPIList(), ctx, windowObject, true);

        //string internal
        _setCustomJS(m_frame->getCustomJSStringAPIListInternal(), ctx, windowObject, true);
    }
#endif // WKC_ENABLE_CUSTOMJS

}

void
FrameLoaderClientWKC::setMainFrameDocumentReady(bool flag)
{
    m_appClient->setMainFrameDocumentReady(flag);
}


void
FrameLoaderClientWKC::startDownload(const WebCore::ResourceRequest& request, const WTF::String& suggestedName, WebCore::FromDownloadAttribute)
{
    ResourceRequestPrivate req(request);
    m_appClient->startDownload(req.wkc(), suggestedName);
}


void
FrameLoaderClientWKC::willChangeTitle(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->willChangeTitle(&ldr.wkc());
}

void
FrameLoaderClientWKC::didChangeTitle(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->didChangeTitle(&ldr.wkc());
}


void
FrameLoaderClientWKC::committedLoad(WebCore::DocumentLoader* loader, const WebCore::SharedBuffer& data)
{
    // Plugin data redirection removed with NPAPI plugin support.
    DocumentLoaderPrivate ldr(loader);
    m_appClient->committedLoad(&ldr.wkc(), reinterpret_cast<const char*>(data.span().data()), static_cast<int>(data.size()));

    WebCore::FrameLoader& frameLoader = m_frame->core()->loader();
    WebCore::DocumentWriter& writer = frameLoader.documentLoader()->writer();
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
    // NPAPI plugin data redirection was removed; forward completion to the app.
    DocumentLoaderPrivate ldr(loader);
    m_appClient->finishedLoading(&ldr.wkc());
}


void
FrameLoaderClientWKC::updateGlobalHistory()
{
    m_appClient->updateGlobalHistory();
}

void
FrameLoaderClientWKC::updateGlobalHistoryRedirectLinks()
{
    m_appClient->updateGlobalHistoryRedirectLinks();
}

WebCore::ShouldGoToHistoryItem
FrameLoaderClientWKC::shouldGoToHistoryItem(WebCore::HistoryItem& item, WebCore::IsSameDocumentNavigation) const
{
    HistoryItemPrivate his(&item);
    return m_appClient->shouldGoToHistoryItem(&his.wkc()) ? WebCore::ShouldGoToHistoryItem::Yes : WebCore::ShouldGoToHistoryItem::No;
}


bool
FrameLoaderClientWKC::shouldFallBack(const WebCore::ResourceError& error) const
{
    ResourceErrorPrivate wobj(error);
    return m_appClient->shouldFallBack(wobj.wkc());
}


bool
FrameLoaderClientWKC::canHandleRequest(const WebCore::ResourceRequest& request) const
{
    ResourceRequestPrivate req(request);
    return m_appClient->canHandleRequest(req.wkc());
}

bool
FrameLoaderClientWKC::canShowMIMEType(const WTF::String& type) const
{
    return m_appClient->canShowMIMEType(type);
}

bool
FrameLoaderClientWKC::representationExistsForURLScheme(WTF::StringView string) const
{
    return m_appClient->representationExistsForURLScheme(string.toString());
}

WTF::String
FrameLoaderClientWKC::generatedMIMETypeForURLScheme(WTF::StringView string) const
{
    return m_appClient->generatedMIMETypeForURLScheme(string.toString());
}


void
FrameLoaderClientWKC::frameLoadCompleted()
{
    m_appClient->frameLoadCompleted();
}

void
FrameLoaderClientWKC::saveViewStateToItem(WebCore::HistoryItem& item)
{
    HistoryItemPrivate his(&item);
    m_appClient->saveViewStateToItem(&his.wkc());
}

void
FrameLoaderClientWKC::restoreViewState()
{
    m_appClient->restoreViewState();
}

void
FrameLoaderClientWKC::provisionalLoadStarted()
{
    m_appClient->provisionalLoadStarted();
}

void
FrameLoaderClientWKC::didFinishLoad()
{
    m_appClient->didFinishLoad();
}

void
FrameLoaderClientWKC::prepareForDataSourceReplacement()
{
    m_appClient->prepareForDataSourceReplacement();
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
    // This function is for History

    WKC::String str(title.string);
    str.setDirection(title.direction==WebCore::TextDirection::LTR ? WKC::LTR : WKC::RTL);
    m_appClient->setTitle(str, uri);
}


WTF::String
FrameLoaderClientWKC::userAgent(const WTF::URL& uri) const
{
    return m_appClient->userAgent(uri);
}


void
FrameLoaderClientWKC::savePlatformDataToCachedFrame(WebCore::CachedFrame* frame)
{
    CachedFramePrivate c(frame);
    m_appClient->savePlatformDataToCachedFrame(&c.wkc());
}

void
FrameLoaderClientWKC::transitionToCommittedFromCachedFrame(WebCore::CachedFrame* frame)
{
    CachedFramePrivate c(frame);
    m_appClient->transitionToCommittedFromCachedFrame(&c.wkc());
}

void
FrameLoaderClientWKC::transitionToCommittedForNewPage(InitializingIframe)
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    int screenWidth = 0;
    if (m_frame->core()->view()) {
        screenWidth = m_frame->core()->view()->screenWidth();
    }
#endif

    WKCWebViewPrivate* containingWindow = m_frame->m_view;
    WebCore::IntSize desktopsize = containingWindow->defaultDesktopSize();
    WebCore::IntSize viewsize = containingWindow->defaultViewSize();
    bool transparent = containingWindow->transparent();
    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparentBlack : WebCore::Color::white;
    WebCore::LocalFrame* frame = m_frame->core();
    bool usefixed = false;
    if (frame->view()) {
        usefixed = frame->view()->useFixedLayout();
    }

    WebCore::ScrollbarMode scrollbarmode = frame->ownerElement() ? WebCore::ScrollbarMode::Auto : WebCore::ScrollbarMode::AlwaysOff;
    frame->createView(desktopsize, backgroundColor, viewsize, usefixed, scrollbarmode, false, scrollbarmode, false);

    if (frame->view()) {
        if (!frame->ownerElement()) {
            frame->view()->setCanHaveScrollbars(false);
        }
    }

#if ENABLE(WKC_ANDROID_LAYOUT)
    if (frame->view()) {
        frame->view()->setScreenWidth(screenWidth);
    }
#endif

    m_appClient->transitionToCommittedForNewPage();
}


bool
FrameLoaderClientWKC::canCachePage() const
{
    return m_appClient->canCachePage();
}

void
FrameLoaderClientWKC::convertMainResourceLoadToDownload(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
{
    // Single-process WKC downloads are driven through WKCDownload / the peer
    // layer rather than a main-resource-load conversion, so nothing to do here.
}

RefPtr<WebCore::Widget>
FrameLoaderClientWKC::createPlugin(WebCore::HTMLPlugInElement&, const WTF::URL&, const Vector<AtomString>&, const Vector<AtomString>&, const WTF::String&, bool)
{
    // NPAPI/Netscape plugins are not supported on Wii U.
    return nullptr;
}

void
FrameLoaderClientWKC::redirectDataToPlugin(WebCore::Widget&)
{
    // No plugin support; nothing to redirect.
}

void
FrameLoaderClientWKC::sendH2Ping(const WTF::URL& url, CompletionHandler<void(Expected<Seconds, WebCore::ResourceError>&&)>&& completionHandler)
{
    completionHandler(makeUnexpected(WebCore::internalError(url)));
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForResponse(const WebCore::ResourceResponse& response, const WebCore::ResourceRequest& request, const WTF::String&, WebCore::FramePolicyFunction&& function)
{
    auto* heapFunc = new WebCore::FramePolicyFunction(WTFMove(function));
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)heapFunc);
    ResourceResponsePrivate res(response);
    ResourceRequestPrivate req(request);
    m_appClient->dispatchDecidePolicyForResponse(*f, res.wkc(), req.wkc());
}

void
FrameLoaderClientWKC::dispatchWillSendSubmitEvent(Ref<WebCore::FormState>&& state)
{
    FormStatePrivate st(state.ptr());
    m_appClient->dispatchWillSendSubmitEvent(&st.wkc());
}

bool
FrameLoaderClientWKC::canShowMIMETypeAsHTML(const WTF::String& MIMEType) const
{
    return m_appClient->canShowMIMETypeAsHTML(MIMEType);
}

WebCore::ObjectContentType
FrameLoaderClientWKC::objectContentType(const WTF::URL& url, const WTF::String& mime)
{
    WKC::ObjectContentType ret = m_appClient->objectContentType(url, mime, false);
    switch (ret) {
    case WKC::ObjectContentImage:
        return WebCore::ObjectContentType::Image;
    case WKC::ObjectContentFrame:
        return WebCore::ObjectContentType::Frame;
    case WKC::ObjectContentNetscapePlugin:
        return WebCore::ObjectContentType::PlugIn;
    case WKC::ObjectContentOtherPlugin:
        return WebCore::ObjectContentType::PlugIn;
    case WKC::ObjectContentNone:
    default:
        return WebCore::ObjectContentType::None;
    }
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
    m_appClient->didChangeScrollOffset();
}

bool
FrameLoaderClientWKC::allowScript(bool enabledPerSettings)
{
    return m_appClient->allowScript(enabledPerSettings);
}

bool
FrameLoaderClientWKC::shouldForceUniversalAccessFromLocalURL(const WTF::URL& url)
{
    return m_appClient->shouldForceUniversalAccessFromLocalURL(url);
}

void
FrameLoaderClientWKC::dispatchGlobalObjectAvailable(WebCore::DOMWrapperWorld& world)
{
    DOMWrapperWorldPrivate w(&world);
    m_appClient->dispatchGlobalObjectAvailable(&w.wkc());
}

// framePolicyFunction

FramePolicyFunction::FramePolicyFunction(void* parent, void* func)
    : m_parent(parent)
    , m_func(func)
{
}

FramePolicyFunction::~FramePolicyFunction()
{
}

void
FramePolicyFunction::reply(WKC::PolicyAction action)
{
    // FramePolicyFunction is a CompletionHandler<void(PolicyAction)> in modern
    // WebKit; it is heap-allocated by the adapter so it outlives the callback,
    // so invoke it directly and then destroy it.
    WebCore::FramePolicyFunction* func = (WebCore::FramePolicyFunction *)m_func;
    (*func)((WebCore::PolicyAction)action);
    delete func;
    delete this;
}

} // namespace
