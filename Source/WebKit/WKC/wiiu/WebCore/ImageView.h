/*
 *  ImageView.h
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

#ifndef ImageView_h
#define ImageView_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {
    class Frame;

    class ImageView : public RefCounted<ImageView> {
    public:
        static PassRefPtr<ImageView> create(Frame* frame) { return adoptRef(new ImageView(frame)); }
        virtual ~ImageView();
        void disconnectFrame() { m_frame = 0; }

        bool end();
        long getErrorCode() const;

        // getter/setter functions
        long viewMode() const;
        void setViewMode(long mode) const;

    private:
        ImageView();
        ImageView(Frame*);
        bool chromeVisible() const;

        Frame* m_frame;
    };

} // namespace WebCore

#endif // ImageView_h

