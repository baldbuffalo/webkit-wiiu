/*
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
 *
 * [license unchanged]
 */
#pragma once

#include <wtf/text/WTFString.h>
#include <openssl/ossl_typ.h>

namespace WebCore {

class ClientCertificate {
public:
    ClientCertificate();
    ~ClientCertificate();

    static ClientCertificate* create(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len);
    static ClientCertificate* createByDER(const unsigned char* cert, int cert_len, const unsigned char* key, int key_len);

    const String& issuer() const { return m_Issuer; }
    const String& subject() const { return m_Subject; }
    const String& notbefore() const { return m_NotBefore; }
    const String& notafter() const { return m_NotAfter; }
    const String& serialnumber() const { return m_serialNumber; }

    bool sameIssuer(const char* s) { return m_Issuer == String::fromLatin1(s); }

    void* pkcs12() { return m_pkcs12; }
    int   pkcs12len() { return m_pkcs12Len; }
    void* pass() { return m_pass; }
    int   passlen() { return m_passLen; }
    X509* cert() { return m_cert; }
    void* privateKey() { return m_privateKey; }
    int   privateKeylen() { return m_privateKeyLen; }

    void setCAIndexs(void** caindexs, int num) { m_caIndexs = caindexs; m_caIndexNum = num; }

private:
    void init(X509* x509);

    String m_Issuer;
    String m_Subject;
    String m_NotBefore;
    String m_NotAfter;
    String m_serialNumber;

    void* m_pkcs12 { nullptr };
    int   m_pkcs12Len { 0 };
    void* m_pass { nullptr };
    int   m_passLen { 0 };
    X509* m_cert { nullptr };
    void* m_privateKey { nullptr };
    int   m_privateKeyLen { 0 };
    void** m_caIndexs { nullptr };
    int    m_caIndexNum { 0 };
};

} // namespace WebCore
