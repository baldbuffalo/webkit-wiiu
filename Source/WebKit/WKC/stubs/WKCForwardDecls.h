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

} // namespace WebCore
