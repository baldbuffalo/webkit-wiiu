/*
 *  Wiiu.h
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

#ifndef Wiiu_h
#define Wiiu_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WebCore {
    class Frame;
    class Remote;
    class WiiuGamePad;
    class VideoPlayer;
    class ImageView;

    class Wiiu : public RefCounted<Wiiu> {
    public:
        static PassRefPtr<Wiiu> create(Frame* frame) { return adoptRef(new Wiiu(frame)); }
        virtual ~Wiiu();
        void disconnectFrame();

        Remote* remote() const;
        WiiuGamePad* gamepad() const;
        VideoPlayer* videoplayer() const;
        ImageView* imageview() const;

    private:
        Wiiu();
        Wiiu(Frame*);
        Frame* m_frame;
        mutable RefPtr<Remote> m_remote;
        mutable RefPtr<WiiuGamePad> m_gamepad;
        mutable RefPtr<VideoPlayer> m_videoPlayer;
        mutable RefPtr<ImageView> m_imageView;
    };

} // namespace WebCore

#endif // Wiiu_h
