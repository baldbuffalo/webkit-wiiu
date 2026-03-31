#pragma once
#ifndef U_SHOW_CPLUSPLUS_API
#define U_SHOW_CPLUSPLUS_API 0
#endif
#include "QualifiedName.h"

namespace WebCore {

class HTMLQualifiedName : public QualifiedName {
public:
    using QualifiedName::QualifiedName;
};

class HTMLSelectedContentElement;
class HTMLOptGroupElement;
class HTMLMeterElement;

} // namespace WebCore

namespace WTF {

template<> struct TypeCastTraits<const WebCore::HTMLMeterElement, const WebCore::Element, false> {
    static bool isOfType(const WebCore::Element&);
};
template<> struct TypeCastTraits<WebCore::HTMLMeterElement, WebCore::Element, false>
    : TypeCastTraits<const WebCore::HTMLMeterElement, const WebCore::Element, false> { };

} // namespace WTF
