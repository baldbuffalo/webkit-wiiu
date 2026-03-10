/*
 *  Wiiu.cpp
 *
 *  Copyright(c) 2012-2014 ACCESS CO., LTD. All rights reserved.
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
#include "Wiiu.h"

#include "Frame.h"
#include "Remote.h"
#include "WiiuGamePad.h"
#include "VideoPlayer.h"
#include "ImageView.h"

namespace WebCore {

Wiiu::Wiiu(Frame* frame)
    : m_frame(frame)
{
}

Wiiu::~Wiiu()
{
}

void
Wiiu::disconnectFrame()
{
    m_frame = 0; 
    if (m_videoPlayer)
        m_videoPlayer->disconnectFrame();
    if (m_imageView)
        m_imageView->disconnectFrame();
}

Remote*
Wiiu::remote() const
{
    if( !m_remote )
        m_remote = Remote::create();
    return m_remote.get();
}

WiiuGamePad*
Wiiu::gamepad() const
{
    if(!m_gamepad)
        m_gamepad = WiiuGamePad::create();
    return m_gamepad.get();
}

VideoPlayer*
Wiiu::videoplayer() const
{
    if(!m_videoPlayer)
        m_videoPlayer = VideoPlayer::create(m_frame);
    return m_videoPlayer.get();
}

ImageView*
Wiiu::imageview() const
{
    if(!m_imageView)
        m_imageView = ImageView::create(m_frame);
    return m_imageView.get();
}
} //namespace
