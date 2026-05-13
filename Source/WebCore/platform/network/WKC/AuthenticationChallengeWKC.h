/* 
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
 *
 * [license unchanged]
 */
#pragma once
#include "AuthenticationChallenge.h"
#include "AuthenticationClient.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class ResourceHandle;

class AuthenticationChallengeWKC : public AuthenticationChallenge {
public:
    AuthenticationChallengeWKC();
    AuthenticationChallengeWKC(const ProtectionSpace&, const Credential&, unsigned previousFailureCount, const ResourceResponse&, const ResourceError&);
    AuthenticationChallengeWKC(const ProtectionSpace&, const Credential&, unsigned previousFailureCount, const ResourceResponse&, const ResourceError&, ResourceHandle*);

    ResourceHandle* sourceHandle() const { return m_sourceHandle.get(); }
    AuthenticationClient* authenticationClient() const { return m_authenticationClient.get(); }

    RefPtr<AuthenticationClient> m_authenticationClient;

private:
    RefPtr<ResourceHandle> m_sourceHandle;
};

} // namespace WebCore
