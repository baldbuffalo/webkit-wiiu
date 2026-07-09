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

#include "helpers/WKCDeviceOrientation.h"
#include "helpers/privates/WKCDeviceOrientationPrivate.h"

#include "DeviceOrientationData.h"

#include <memory>
#include <optional>

namespace WKC {

// Modern WebCore::DeviceOrientationData represents "not provided" as std::nullopt
// rather than a paired canProvide flag; translate the (still app-facing) WKC
// canProvide/value pairs at the boundary.
static inline std::optional<double> optionalValue(bool canProvide, double value)
{
    return canProvide ? std::optional<double> { value } : std::nullopt;
}

// Private Implementation

// DeviceOrientationPrivate

DeviceOrientationPrivate::DeviceOrientationPrivate(DeviceOrientation* parent, bool canProvideAlpha, double alpha, bool canProvideBeta, double beta, bool canProvideGamma, double gamma, bool canProvideAbsolute, bool absolute)
     : m_webcore(WebCore::DeviceOrientationData::create(
           optionalValue(canProvideAlpha, alpha),
           optionalValue(canProvideBeta, beta),
           optionalValue(canProvideGamma, gamma),
           canProvideAbsolute ? std::optional<bool> { absolute } : std::nullopt))
     , m_wkc(parent)
{
}

DeviceOrientationPrivate::~DeviceOrientationPrivate() = default;


// Implementation

// DeviceOrientation

DeviceOrientation::DeviceOrientation(bool canProvideAlpha, double alpha, bool canProvideBeta, double beta, bool canProvideGamma, double gamma, bool canProvideAbsolute, bool absolute)
     : m_private(std::make_unique<DeviceOrientationPrivate>(this, canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma, canProvideAbsolute, absolute))
{
}

DeviceOrientation::~DeviceOrientation() = default;

DeviceOrientation*
DeviceOrientation::create(bool canProvideAlpha, double alpha, bool canProvideBeta, double beta, bool canProvideGamma, double gamma, bool canProvideAbsolute, bool absolute)
{
    return new DeviceOrientation(canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma, canProvideAbsolute, absolute);
}

void
DeviceOrientation::destroy(DeviceOrientation* self)
{
    delete self;
}

} // namespace
