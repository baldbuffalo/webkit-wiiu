/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include "DeviceOrientationClientWKC.h"

#include "WKCWebViewPrivate.h"

#include "helpers/DeviceOrientationClientIf.h"

#include "helpers/WKCDeviceOrientation.h"
#include "helpers/privates/WKCDeviceOrientationPrivate.h"
#include "helpers/privates/WKCDeviceOrientationControllerPrivate.h"

namespace WKC {

DeviceOrientationClientWKC::DeviceOrientationClientWKC(WKCWebViewPrivate* view)
     : m_view(view)
     , m_appClient(nullptr)
{
}

DeviceOrientationClientWKC::~DeviceOrientationClientWKC()
{
    // m_orientation and m_controller are std::unique_ptr -- freed automatically.
    if (m_appClient) {
        m_view->clientBuilders().deleteDeviceOrientationClient(m_appClient);
        m_appClient = nullptr;
    }
}

DeviceOrientationClientWKC*
DeviceOrientationClientWKC::create(WKCWebViewPrivate* view)
{
    DeviceOrientationClientWKC* self = 0;
    self = new DeviceOrientationClientWKC(view);
    if (!self) {
        return 0;
    }
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
DeviceOrientationClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createDeviceOrientationClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
DeviceOrientationClientWKC::setController(WebCore::DeviceOrientationController* controller)
{
    if (!m_controller || m_controller->webcore() != controller)
        m_controller = std::make_unique<DeviceOrientationControllerPrivate>(controller);
    m_appClient->setController(&m_controller->wkc());
}

void
DeviceOrientationClientWKC::startUpdating()
{
    m_appClient->startUpdating();
}

void
DeviceOrientationClientWKC::stopUpdating()
{
    m_appClient->stopUpdating();
}

WebCore::DeviceOrientationData*
DeviceOrientationClientWKC::lastOrientation() const
{
    // Adopt the app-provided orientation snapshot; the previous one is released by reset().
    m_orientation.reset(m_appClient->lastOrientation());
    if (!m_orientation)
        return nullptr;
    return m_orientation->priv()->webcore().get();
}

void
DeviceOrientationClientWKC::deviceOrientationControllerDestroyed()
{
    delete this;
}

} // namespace WKC
