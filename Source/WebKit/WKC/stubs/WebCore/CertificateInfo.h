#pragma once

// WKC uses curl, not soup — stub out CertificateInfo for bare metal
namespace WebCore {

class CertificateInfo {
public:
    CertificateInfo() = default;
    bool isEmpty() const { return true; }
};

} // namespace WebCore
