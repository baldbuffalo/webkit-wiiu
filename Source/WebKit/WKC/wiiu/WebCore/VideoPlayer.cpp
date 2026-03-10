/*
 * VideoPlayer.cpp
 *
 * Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
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
#include "VideoPlayer.h"
#include "private/wkcwiiuvideoplayerprivate.h"

#include "Chrome.h"
#include "Frame.h"
#include "Page.h"

namespace WebCore {

VideoPlayer::VideoPlayer(Frame* frame)
    : m_frame(frame)
{
}

bool
VideoPlayer::end()
{
    if (!chromeVisible())
        return false;
    return WKC::WKCWiiuEndVideoPlayer();
}

long
VideoPlayer::viewMode() const
{
    return WKC::WKCWiiuVideoPlayerViewMode();
}

void
VideoPlayer::setViewMode(long viewMode) const
{
    if (!chromeVisible())
        return;

    // reject invalid viewMode
    if (viewMode < 0 || 1 < viewMode) {
        return;
    }

    WKC::WKCWiiuVideoPlayerSetViewMode(viewMode);
}

bool
VideoPlayer::chromeVisible() const
{
    if (!m_frame || !m_frame->page() || !m_frame->page()->chrome() || !m_frame->page()->chrome()->chromeVisible())
        return false;

    return true;
}

} //namespace
