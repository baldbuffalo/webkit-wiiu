#pragma once
#include "RenderStyle.h"
#include "RenderStyleConstants.h"
#include "FontCascade.h"
#include "FontDescription.h"

namespace WebCore {

inline bool RenderStyle::evaluationTimeZoomEnabled() const
{
    return true;
}

inline float RenderStyle::usedZoom() const
{
    return effectiveZoom();
}

inline float RenderStyle::computedFontSize() const
{
    return fontDescription().computedSize();
}

inline Visibility RenderStyle::usedVisibility() const
{
    return Visibility::Visible;
}

inline bool RenderStyle::hasBackground() const
{
    return true;
}

inline bool RenderStyle::hasOutline() const
{
    return false;
}

inline bool RenderStyle::hasTransformRelatedProperty() const
{
    return false;
}

inline StyleAppearance RenderStyle::usedAppearance() const
{
    return static_cast<StyleAppearance>(specifiedAppearance());
}

inline const Style::MaskLayers& RenderStyle::maskLayers() const
{
    static const Style::MaskLayers empty;
    return empty;
}

} // namespace WebCore
