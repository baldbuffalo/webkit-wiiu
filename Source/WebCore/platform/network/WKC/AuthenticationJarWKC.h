/* 
 * Copyright (c) 2012,2013 ACCESS CO., LTD. All rights reserved.
 *
 * [license unchanged]
 */
#pragma once

#include <wtf/URL.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>
#include "ResourceHandle.h"
#include "ResourceHandleInternalWKC.h"

namespace WebCore {

class AuthenticationJar {
public:
    AuthenticationJar();
    ~AuthenticationJar();

    AuthenticationJar* get() { return this; }

    bool Challenge(ResourceHandle* job, long httpCode);
    bool getWebUserPassword(String url, ProtectionSpaceServerType&, ProtectionSpaceAuthenticationScheme&, String realm, String& user, String& passwd);
    void setWebUserPassword(String url, ProtectionSpaceServerType, ProtectionSpaceAuthenticationScheme, String realm, String user, String passwd, String location = String(), bool confirmed = false);
    void deleteWebUserPassword(String url, String realm);
    bool getProxyUserPassword(String url, int port, ProtectionSpaceServerType&, ProtectionSpaceAuthenticationScheme&, String& user, String& passwd);
    void setProxyUserPassword(String url, int port, ProtectionSpaceServerType, ProtectionSpaceAuthenticationScheme, String user, String passwd);
    void deleteProxyUserPassword(String url, int port);

private:
    class AuthInfo {
    public:
        AuthInfo()
            : m_serverType(ProtectionSpaceServerHTTP)
            , m_authScheme(ProtectionSpaceAuthenticationSchemeDefault)
            , m_port(0)
            , m_confirmed(false)
        {}
        ~AuthInfo() = default;

        ProtectionSpaceServerType           m_serverType;
        ProtectionSpaceAuthenticationScheme m_authScheme;
        String m_host;
        unsigned short m_port;
        String m_basePath;
        String m_fullPath;
        String m_realm;
        bool   m_confirmed;
        String m_user;
        String m_passwd;
        String m_tmpUser;
        String m_tmpPasswd;
    };

    Vector<AuthInfo*> m_WebAuthInfoList;
    Vector<AuthInfo*> m_ProxyAuthInfoList;

    AuthInfo* longer(AuthInfo* current, AuthInfo* newinfo, bool compFull);
};

} // namespace WebCore
