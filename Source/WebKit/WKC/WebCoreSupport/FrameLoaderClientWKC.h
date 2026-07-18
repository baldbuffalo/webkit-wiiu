/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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

#ifndef FrameLoaderClientWKC_h
#define FrameLoaderClientWKC_h

#include "LocalFrameLoaderClient.h"
#include "FrameNetworkingContext.h"
#include "ResourceResponse.h"
#include "ResourceLoaderIdentifier.h"
#include "BackForwardItemIdentifier.h"
#include "FrameLoaderTypes.h"
#include "WKCEnums.h"
#include "WTFString.h"
#include <optional>
#include <wtf/CompletionHandler.h>
#include <wtf/Expected.h>
#include <wtf/Seconds.h>
#include <wtf/WallTime.h>
#include <wtf/text/StringView.h>

#ifdef WKC_ENABLE_CUSTOMJS
#include <wkc/wkccustomjs.h>
#endif // WKC_ENABLE_CUSTOMJS

namespace WebCore {
class HistoryItem;
class AuthenticationChallenge;
class SubstituteData;
class SocketStreamHandle;
class CachedFrame;
}

namespace WKC {

class FrameLoaderClientIf;
class WKCWebFramePrivate;
class WKCWebFrame;
class WKCPolicyDecision;

class FrameLoaderClientWKC : public WebCore::LocalFrameLoaderClient {
public:
    // Modern WebKit builds the loader client through a ClientCreator lambda that
    // receives the LocalFrame and its FrameLoader, so the client is constructed
    // with the FrameLoader the base class requires.
    FrameLoaderClientWKC(WebCore::FrameLoader&, WKCWebFramePrivate*);
    virtual ~FrameLoaderClientWKC();

    bool construct();

    WKCWebFrame* webFrame() const;

    //
    // Inheritance FrameLoaderClient
    //
    /* hasHTMLView() */

    virtual bool hasWebView() const;

    // Modern LocalFrameLoaderClient additions. On a single-process embedder like
    // WKC these coordinate cross-process/site-isolation cases that cannot occur,
    // so they are correct as no-ops (or the local answer).
    void willReplaceMultipartContent() override { }
    void didReplaceMultipartContent() override { }
    void didRestoreFromBackForwardCache() override { }
    void dispatchGoToBackForwardItemAtIndex(int, WebCore::FrameLoadType) override { }
    void dispatchLoadEventToOwnerElementInAnotherProcess() override { }
    void loadStorageAccessQuirksIfNeeded() override { }
    void updateCachedDocumentLoader(WebCore::DocumentLoader&) override { }
    bool supportsAsyncShouldGoToHistoryItem() const override { return false; }
    // Not inline: returns RefPtr<HistoryItem>, whose destructor needs the
    // complete type, and this header only forward-declares HistoryItem.
    RefPtr<WebCore::HistoryItem> createHistoryItemTree(bool, WebCore::BackForwardItemIdentifier) const override;
    void shouldGoToHistoryItemAsync(WebCore::HistoryItem&, CompletionHandler<void(WebCore::ShouldGoToHistoryItem)>&& completionHandler) const override { completionHandler(WebCore::ShouldGoToHistoryItem::Yes); }

    void makeRepresentation(WebCore::DocumentLoader*) override;
    void forceLayoutForNonHTML() override;
    void setCopiesOnScroll() override;
    void detachedFromParent2() override;
    void detachedFromParent3() override;
    void assignIdentifierToInitialRequest(WebCore::ResourceLoaderIdentifier identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&) override;
    void dispatchWillSendRequest(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse) override;
    bool shouldUseCredentialStorage(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier) override;
    void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, const WebCore::AuthenticationChallenge&) override;
    void dispatchDidReceiveResponse(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceResponse&) override;
    void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, int lengthReceived) override;
    void dispatchDidFinishLoading(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier) override;
    void dispatchDidFailLoading(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceError&) override;
    bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length) override;

    void dispatchDidDispatchOnloadEvents() override;
    void dispatchDidReceiveServerRedirectForProvisionalLoad() override;
    void dispatchDidCancelClientRedirect() override;
    void dispatchWillPerformClientRedirect(const WTF::URL&, double interval, WallTime fireDate, WebCore::LockBackForwardList) override;
    void dispatchDidNavigateWithinPage() override;
    void dispatchDidChangeLocationWithinPage() override;
    void dispatchDidPushStateWithinPage() override;
    void dispatchDidReplaceStateWithinPage() override;
    void dispatchDidPopStateWithinPage() override;
    void dispatchWillClose() override;
    void dispatchDidReceiveIcon() override;
    void dispatchDidStartProvisionalLoad() override;
    void dispatchDidReceiveTitle(const WebCore::StringWithDirection&) override;
    void dispatchDidCommitLoad(std::optional<WebCore::HasInsecureContent>, std::optional<WebCore::UsedLegacyTLS>, std::optional<WebCore::WasPrivateRelayed>) override;
    void dispatchDidFailProvisionalLoad(const WebCore::ResourceError&, WebCore::WillContinueLoading, WebCore::WillInternallyHandleFailure) override;
    void dispatchDidFailLoad(const WebCore::ResourceError&) override;
    void dispatchDidFinishDocumentLoad() override;
    void dispatchDidFinishLoad() override;
    void dispatchDidLayout() override;

    WebCore::LocalFrame* dispatchCreatePage(const WebCore::NavigationAction&, WebCore::NewFrameOpenerPolicy, const WTF::String& openedMainFrameName) override;
    void dispatchShow() override;

    void dispatchDecidePolicyForResponse(const WebCore::ResourceResponse&, const WebCore::ResourceRequest&, const WTF::String& downloadAttribute, WebCore::FramePolicyFunction&&) override;
    void dispatchDecidePolicyForNewWindowAction(const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WebCore::FormState*, const WTF::String& frameName, std::optional<WebCore::HitTestResult>&&, WebCore::FramePolicyFunction&&) override;
    void dispatchDecidePolicyForNavigationAction(const WebCore::NavigationAction&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse, WebCore::FormState*, const WTF::String& clientRedirectSourceForHistory, std::optional<WebCore::NavigationIdentifier>, std::optional<WebCore::HitTestResult>&&, bool hasOpener, WebCore::NavigationUpgradeToHTTPSBehavior, WebCore::SandboxFlags, WebCore::PolicyDecisionMode, WebCore::FramePolicyFunction&&) override;
    void cancelPolicyCheck() override;
    void dispatchUnableToImplementPolicy(const WebCore::ResourceError&) override;

    // FrameLoaderClient (base) cross-process/site-isolation hooks. WKC is a
    // single-process embedder, so these are correct as no-ops.
    void updateSandboxFlags(WebCore::SandboxFlags) override { }
    void updateOpener(std::optional<WebCore::FrameIdentifier>) override { }
    void setPrinting(bool, WebCore::FloatSize, WebCore::FloatSize, float, WebCore::AdjustViewSize) override { }

    void dispatchWillSendSubmitEvent(Ref<WebCore::FormState>&&) override;
    void dispatchWillSubmitForm(WebCore::FormState&, WTF::URL&& requestURL, WTF::String&& method, CompletionHandler<void()>&&) override;

    void revertToProvisionalState(WebCore::DocumentLoader*) override;
    void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&) override;

    void setMainFrameDocumentReady(bool) override;

    void startDownload(const WebCore::ResourceRequest&, const WTF::String& suggestedName, WebCore::FromDownloadAttribute) override;

    void willChangeTitle(WebCore::DocumentLoader*) override;
    void didChangeTitle(WebCore::DocumentLoader*) override;

    void committedLoad(WebCore::DocumentLoader*, const WebCore::SharedBuffer&) override;
    void finishedLoading(WebCore::DocumentLoader*) override;

    void updateGlobalHistory() override;
    void updateGlobalHistoryRedirectLinks() override;
    WebCore::ShouldGoToHistoryItem shouldGoToHistoryItem(WebCore::HistoryItem&, WebCore::IsSameDocumentNavigation) const override;

    bool shouldFallBack(const WebCore::ResourceError&) const override;

    bool canHandleRequest(const WebCore::ResourceRequest&) const override;
    bool canShowMIMEType(const WTF::String&) const override;
    bool canShowMIMETypeAsHTML(const WTF::String& MIMEType) const override;
    bool representationExistsForURLScheme(WTF::StringView) const override;
    WTF::String generatedMIMETypeForURLScheme(WTF::StringView) const override;

    void frameLoadCompleted() override;
    void saveViewStateToItem(WebCore::HistoryItem&) override;
    void restoreViewState() override;
    void provisionalLoadStarted() override;
    void didFinishLoad() override;
    void prepareForDataSourceReplacement() override;

    Ref<WebCore::DocumentLoader> createDocumentLoader(WebCore::ResourceRequest&&, WebCore::SubstituteData&&, WebCore::ResourceRequest&&) override;
    Ref<WebCore::DocumentLoader> createDocumentLoader(WebCore::ResourceRequest&&, WebCore::SubstituteData&&) override;

    void setTitle(const WebCore::StringWithDirection& title, const WTF::URL&) override;

    WTF::String userAgent(const WTF::URL&) const override;

    void savePlatformDataToCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedForNewPage(InitializingIframe) override;

    bool canCachePage() const override;
    void convertMainResourceLoadToDownload(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&) override;

    RefPtr<WebCore::LocalFrame> createFrame(const AtomString& name, WebCore::HTMLFrameOwnerElement&) override;
    RefPtr<WebCore::Widget> createPlugin(WebCore::HTMLPlugInElement&, const WTF::URL&, const Vector<AtomString>&, const Vector<AtomString>&, const WTF::String&, bool loadManually) override;
    void redirectDataToPlugin(WebCore::Widget&) override;

    WebCore::ObjectContentType objectContentType(const WTF::URL&, const WTF::String& mimeType) override;
    AtomString overrideMediaType() const override;

    void dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld&) override;

    void didChangeScrollOffset() override;

    bool allowScript(bool enabledPerSettings) override;

    bool shouldForceUniversalAccessFromLocalURL(const WTF::URL&) override;

    Ref<WebCore::FrameNetworkingContext> createNetworkingContext() override;

    void dispatchGlobalObjectAvailable(WebCore::DOMWrapperWorld&) override;

    void prefetchDNS(const WTF::String&) override { }
    void sendH2Ping(const WTF::URL&, CompletionHandler<void(Expected<Seconds, WebCore::ResourceError>&&)>&&) override;

    //
    // WKC extension
    //
public:
    bool byWKC(void) { return true; }
    virtual bool notifySSLHandshakeStatus(WebCore::ResourceHandle* handle, int status);
    virtual bool dispatchWillAcceptCookie(bool income, WebCore::ResourceHandle* handle, const WTF::String& url, const WTF::String& firstparty_host, const WTF::String& cookie_domain);
    virtual bool dispatchWillReceiveData(WebCore::ResourceHandle*, int length);
    virtual int  dispatchWillPermitSendRequest(WebCore::ResourceHandle* handle, const char* url, int composition, bool isSync, const WebCore::ResourceResponse& redirectResponse);

#ifdef WKC_ENABLE_CUSTOMJS
    virtual bool dispatchWillCallCustomJS(WKCCustomJSAPIList* api, void** context);
#endif // WKC_ENABLE_CUSTOMJS


private:
    void notifyStatus(WKC::LoadStatus loadStatus);

private:
    WKCWebFramePrivate* m_frame;
    WKC::FrameLoaderClientIf* m_appClient;
    WebCore::ResourceResponse m_response;
    WKCPolicyDecision* m_policyDecision;
};

class FrameNetworkingContextWKC : public WebCore::FrameNetworkingContext
{
public:
    static FrameNetworkingContextWKC* create(WebCore::LocalFrame*);

    WebCore::FrameLoaderClient* frameLoaderClient() const;

    WebCore::LocalFrame* coreFrame(){ return frame(); }

    // NetworkingContext (WKC is neither COCOA nor WIN, so only this is pure)
    bool shouldClearReferrerOnHTTPSToHTTPRedirect() const override { return true; }
    // StorageSessionProvider
    WebCore::NetworkStorageSession* storageSession() const override { return nullptr; }

private:
    FrameNetworkingContextWKC(WebCore::LocalFrame*);
};

} // namespace

#endif // FrameLoaderClientWKC_h
