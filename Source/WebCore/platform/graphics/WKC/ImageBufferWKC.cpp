/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Holger Hans Peter Freyther <zecke@selfish.org>
 * Copyright (C) 2008, 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if !USE(WKC_CAIRO)

#include "ImageBuffer.h"
#include "ImageBufferBackend.h"
#include "BitmapImage.h"
#include "Color.h"
#include "GraphicsContext.h"
#include "ImageWKC.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"
#include "PixelBuffer.h"
#include <wtf/text/WTFString.h>

#include <wkc/wkcgpeer.h>
#include <wkc/wkcmpeer.h>

namespace WebCore {

// ---------------------------------------------------------------------------
// WKC ImageBuffer backend
// ---------------------------------------------------------------------------

class ImageBufferWKCBackend final : public ImageBufferBackend {
public:
    static std::unique_ptr<ImageBufferWKCBackend> create(const Parameters& parameters, const ImageBufferCreationContext&)
    {
        auto backend = std::unique_ptr<ImageBufferWKCBackend>(new ImageBufferWKCBackend(parameters));
        if (!backend->m_offscreen)
            return nullptr;
        return backend;
    }

    static std::unique_ptr<ImageBufferWKCBackend> create(const Parameters& parameters, ImageBufferBackendHandle)
    {
        return nullptr;
    }

    static size_t calculateMemoryCost(const Parameters& parameters)
    {
        return parameters.backendSize.area() * 4;
    }

    ImageBufferWKCBackend(const Parameters& parameters)
        : ImageBufferBackend(parameters)
        , m_offscreen(nullptr)
        , m_dc(nullptr)
        , m_image(nullptr)
        , m_graphicsContext(nullptr)
    {
        IntSize size = parameters.backendSize;
        WKCSize osize = { size.width(), size.height() };
        m_offscreen = wkcOffscreenNewPeer(WKC_OFFSCREEN_TYPE_IMAGEBUF, nullptr, 0, &osize);
        if (!m_offscreen)
            return;

        int rowbytes = 0;
        void* bitmap = wkcOffscreenBitmapPeer(m_offscreen, &rowbytes);

        m_dc = wkcDrawContextNewPeer(m_offscreen);
        if (!m_dc) {
            wkcOffscreenDeletePeer(m_offscreen);
            m_offscreen = nullptr;
            return;
        }

        if (bitmap) {
            m_image = ImageWKC::create(ImageWKC::EColorARGB8888, bitmap, rowbytes, 0, 0, size, false);
        }

        m_graphicsContext = createGraphicsContextWKC(m_dc);
    }

    ~ImageBufferWKCBackend() override
    {
        m_graphicsContext = nullptr;
        if (m_image)
            delete m_image;
        if (m_dc)
            wkcDrawContextDeletePeer(m_dc);
        if (m_offscreen)
            wkcOffscreenDeletePeer(m_offscreen);
    }

    GraphicsContext& context() override
    {
        return *m_graphicsContext;
    }

    void flushContext() override
    {
        if (m_offscreen)
            wkcOffscreenFlushPeer(m_offscreen, WKC_OFFSCREEN_FLUSH_FOR_DRAW);
    }

    RefPtr<NativeImage> copyNativeImage() override
    {
        return nullptr;
    }

    RefPtr<NativeImage> createNativeImageReference() override
    {
        return nullptr;
    }

    void getPixelBuffer(const IntRect&, PixelBuffer&) override { }
    void putPixelBuffer(const PixelBuffer&, const IntRect&, const IntPoint&, AlphaPremultiplication) override { }

    static constexpr bool isAccelerated = false;

    String debugDescription() const override
    {
        return "ImageBufferWKCBackend"_s;
    }

    ImageBufferBackendSharing* toBackendSharing() override { return nullptr; }

private:
    void* m_offscreen;
    void* m_dc;
    ImageWKC* m_image;
    std::unique_ptr<GraphicsContext> m_graphicsContext;
};

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
