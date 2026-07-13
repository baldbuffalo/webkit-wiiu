/*
 * Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"

#include "helpers/WKCResourceResponse.h"
#include "helpers/privates/WKCResourceResponsePrivate.h"

#include "ResourceResponse.h"
#include "CertificateInfo.h"

#include <wtf/URL.h>
#include <wtf/text/WTFString.h>
#include "ResourceRequest.h"

#include <wkc/wkcpeer.h>

#include "helpers/WKCURL.h"
#include "helpers/WKCString.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"

namespace WKC {

ResourceResponsePrivateBase::ResourceResponsePrivateBase()
    : m_resourceHandle(0)
{
}

ResourceResponsePrivateBase::~ResourceResponsePrivateBase()
{
    delete m_resourceHandle;
}

ResourceResponsePrivate::ResourceResponsePrivate(const WebCore::ResourceResponse& parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

ResourceResponsePrivate::~ResourceResponsePrivate()
{
}

ResourceResponsePrivateToCore::ResourceResponsePrivateToCore(const ResourceResponse& wkc)
    : m_instance(new WebCore::ResourceResponse())
    , m_webcore(*m_instance)
    , m_wkc(wkc)
{
}

ResourceResponsePrivateToCore::~ResourceResponsePrivateToCore()
{
    delete m_instance;
}

const URL
ResourceResponsePrivateBase::url() const
{
    return webcore().url();
}

const String
ResourceResponsePrivateBase::mimeType() const
{
    return webcore().mimeType();
}

bool
ResourceResponsePrivateBase::isAttachment() const
{
    return webcore().isAttachment();
}

bool
ResourceResponsePrivateBase::isNull() const
{
    return webcore().isNull();
}

int
ResourceResponsePrivateBase::httpStatusCode() const
{
    return webcore().httpStatusCode();
}

long long
ResourceResponsePrivateBase::expectedContentLength() const
{
    return webcore().expectedContentLength();
}

const String&
ResourceResponsePrivateBase::httpStatusText()
{
    m_httpStatusText = webcore().httpStatusText();
    return m_httpStatusText;
}

const String
ResourceResponsePrivateBase::httpHeaderField(const char* name) const
{
    if (!name)
        return String();
    // httpHeaderField now takes StringView (String converts to it).
    return webcore().httpHeaderField(WTF::String::fromLatin1(name));
}

bool
ResourceResponsePrivateBase::wasCached() const
{
    // wasCached() was removed; derive it from the response source.
    using Source = WebCore::ResourceResponse::Source;
    switch (webcore().source()) {
    case Source::DiskCache:
    case Source::DiskCacheAfterValidation:
    case Source::MemoryCache:
    case Source::MemoryCacheAfterValidation:
        return true;
    default:
        return false;
    }
}

// Security / TLS introspection.
//
// In the legacy port these values were read from WKC's forked libcurl handle.
// Modern WebKit surfaces the equivalent information on the response itself:
// the certificate verification result via certificateInfo() and the TLS
// quality via usedLegacyTLS(). Extended Validation is not tracked upstream,
// so it is delegated to the Wii U SSL peer.

long
ResourceResponsePrivateBase::SSLVerifyOpenSSLResult() const
{
    const auto& info = webcore().certificateInfo();
    return info ? info->verificationError() : 0;
}

long
ResourceResponsePrivateBase::SSLVerifycURLResult() const
{
    // The modern curl port folds the OpenSSL peer and host verification
    // results into a single verificationError, so both map to the same value.
    const auto& info = webcore().certificateInfo();
    return info ? info->verificationError() : 0;
}

int
ResourceResponsePrivateBase::secureState() const
{
    // 0: not secure (no TLS), 1: secure (certificate verified),
    // 2: insecure (certificate verification failed).
    const auto& info = webcore().certificateInfo();
    if (!info || info->isEmpty())
        return 0;
    return info->verificationError() ? 2 : 1;
}

int
ResourceResponsePrivateBase::secureLevel() const
{
    // 0: none, 1: verified over legacy TLS, 2: verified over modern TLS,
    // 3: verified with an Extended Validation certificate.
    if (secureState() != 1)
        return 0;
    if (isEVSSL())
        return 3;
    return webcore().usedLegacyTLS() ? 1 : 2;
}

bool
ResourceResponsePrivateBase::isEVSSL() const
{
    const auto& info = webcore().certificateInfo();
    if (!info || info->verificationError() || info->certificateChain().isEmpty())
        return false;
    const auto& leaf = info->certificateChain().first();
    return wkcSSLIsEVCertificatePeer(leaf.data(), static_cast<int>(leaf.size()));
}

ResourceHandle*
ResourceResponsePrivateBase::resourceHandle()
{
    // ResourceResponse no longer carries its originating ResourceHandle.
    if (m_resourceHandle) {
        delete m_resourceHandle;
    }
    m_resourceHandle = new ResourceHandlePrivate(nullptr);
    return &m_resourceHandle->wkc();

}

ResourceResponse::ResourceResponse()
    : m_private(0)
    , m_owned(true)
{
    ResourceResponsePrivateToCore* i = new ResourceResponsePrivateToCore(*this);
    m_private = reinterpret_cast<ResourceResponsePrivate*>(i);
}

ResourceResponse::ResourceResponse(ResourceResponsePrivate& parent)
    : m_private(&parent)
    , m_owned(false)
{
}

ResourceResponse::~ResourceResponse()
{
    if (m_owned) {
//        delete m_private;
        ResourceResponsePrivateToCore* i = (ResourceResponsePrivateToCore *)m_private;
        delete i;
    }
}

ResourceResponse::ResourceResponse(const ResourceResponse& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_owned = false;
    }
}

ResourceResponse&
ResourceResponse::operator=(const ResourceResponse& other)
{
    if (this!=&other) {
        m_private = other.m_private;
        m_owned = false;
    }
    return *this;
}

const URL
ResourceResponse::url() const
{
    return m_private->url();
}

const String
ResourceResponse::mimeType() const
{
    return m_private->mimeType();
}

bool
ResourceResponse::isAttachment() const
{
    return m_private->isAttachment();
}

bool
ResourceResponse::isNull() const
{
    return m_private->isNull();
}

int
ResourceResponse::httpStatusCode() const
{
    return m_private->httpStatusCode();
}

ResourceHandle*
ResourceResponse::resourceHandle() const
{
    return m_private->resourceHandle();
}

long long
ResourceResponse::expectedContentLength() const
{
    return m_private->expectedContentLength();
}

const String&
ResourceResponse::httpStatusText() const
{
    return m_private->httpStatusText();
}

const String
ResourceResponse::httpHeaderField(const char* name) const
{
    return m_private->httpHeaderField(name);
}

bool
ResourceResponse::wasCached() const
{
    return m_private->wasCached();
}

long
ResourceResponse::SSLVerifyOpenSSLResult() const
{
    return m_private->SSLVerifyOpenSSLResult();
}

long
ResourceResponse::SSLVerifycURLResult() const
{
    return m_private->SSLVerifycURLResult();
}

int
ResourceResponse::secureState() const
{
    return m_private->secureState();
}

int
ResourceResponse::secureLevel() const
{
    return m_private->secureLevel();
}

bool
ResourceResponse::isEVSSL() const
{
    return m_private->isEVSSL();
}

} // namespace

