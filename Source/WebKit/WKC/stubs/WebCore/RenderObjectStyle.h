#pragma once
#include "RenderObject.h"
#include "RenderElement.h"

namespace WebCore {

inline const RenderStyle& RenderObject::style() const
{
    if (isText())
        return parent()->style();
    return downcast<RenderElement>(*this).style();
}

} // namespace WebCore
