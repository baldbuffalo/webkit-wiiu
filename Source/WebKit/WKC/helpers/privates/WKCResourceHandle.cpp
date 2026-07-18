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

#include "helpers/WKCResourceHandle.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"

#include "ResourceHandle.h"
#include "helpers/WKCAuthenticationChallenge.h"
#include "helpers/WKCCredential.h"

#include "helpers/privates/WKCCredentialPrivate.h"
#include "helpers/privates/WKCAuthenticationChallengePrivate.h"


namespace WKC {
ResourceHandlePrivate::ResourceHandlePrivate(WebCore::ResourceHandle* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

ResourceHandlePrivate::~ResourceHandlePrivate()
{
}

void
ResourceHandlePrivate::receivedCredential(const AuthenticationChallengePrivate& auth, const CredentialPrivate& cred)
{
    m_webcore->receivedCredential(auth.webcore(), cred.webcore());
}

void
ResourceHandlePrivate::receivedRequestToContinueWithoutCredential(const AuthenticationChallengePrivate& auth)
{
    m_webcore->receivedRequestToContinueWithoutCredential(auth.webcore());
}

ResourceHandle::ResourceHandle(ResourceHandlePrivate& parent)
    : m_private(parent)
{
}

ResourceHandle::~ResourceHandle()
{
}

void
ResourceHandle::receivedCredential(const AuthenticationChallenge& challenge, const Credential& cred)
{
    m_private.receivedCredential(challenge.priv(), *cred.priv());
}

void
ResourceHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge& challenge)
{
    m_private.receivedRequestToContinueWithoutCredential(challenge.priv());
}

} // namespace
