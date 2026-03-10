/*
 *  ImageView.cpp
 *
 *  Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
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
#include "ImageView.h"
#include "private/wkcwiiuimageviewprivate.h"

#include "Chrome.h"
#include "Frame.h"
#include "Page.h"

namespace WebCore {

ImageView::ImageView(Frame* frame)
    : m_frame(frame)
{
}

ImageView::~ImageView()
{
}

bool
ImageView::end()
{
    if (!chromeVisible())
        return false;
    return WKC::WKCWiiuEndImageView();
}

long
ImageView::viewMode() const
{
    return WKC::WKCWiiuImageViewViewMode();
}

void
ImageView::setViewMode(long mode) const
{
    if (!chromeVisible())
        return;

    // reject invalid mode
    if (mode < 0 || 1 < mode) {
        return;
    }

    WKC::WKCWiiuImageViewChangeViewMode(mode);
}

long
ImageView::getErrorCode() const
{
    return WKC::WKCWiiuImageViewGetLastError();
}


bool
ImageView::chromeVisible() const
{
    if (!m_frame || !m_frame->page() || !m_frame->page()->chrome() || !m_frame->page()->chrome()->chromeVisible())
        return false;

    return true;
}


} //namespace
