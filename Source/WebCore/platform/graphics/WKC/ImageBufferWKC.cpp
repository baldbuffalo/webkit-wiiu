/*
 * Copyright (c) 2010-2013 ACCESS CO., LTD. All rights reserved.
 * [license header omitted for brevity]
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

// Forward declaration from GraphicsContextWKC.cpp
std::unique_ptr<GraphicsContext> createGraphicsContextWKC(void* drawContext);

class ImageBufferWKCBackend final : public ImageBufferBackend {
public:
    static std::unique_ptr<ImageBufferWKCBackend> create(const Parameters& parameters, const ImageBufferCreationContext&)
    {
        auto backend = std::unique_ptr<ImageBufferWKCBackend>(new ImageBufferWKCBackend(parameters));
        if (!backend->m_offscreen)
            return nullptr;
        return backend;
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
        , m_rowbytes(0)
    {
        IntSize size = parameters.backendSize;
        WKCSize osize = { size.width(), size.height() };
        m_offscreen = wkcOffscreenNewPeer(WKC_OFFSCREEN_TYPE_IMAGEBUF, nullptr, 0, &osize);
        if (!m_offscreen)
            return;

        void* bitmap = wkcOffscreenBitmapPeer(m_offscreen, &m_rowbytes);

        m_dc = wkcDrawContextNewPeer(m_offscreen);
        if (!m_dc) {
            wkcOffscreenDeletePeer(m_offscreen);
            m_offscreen = nullptr;
            return;
        }

        if (bitmap)
            m_image = ImageWKC::create(ImageWKC::EColorARGB8888, bitmap, m_rowbytes, size, false);

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

    RefPtr<NativeImage> copyNativeImage() override { return nullptr; }
    RefPtr<NativeImage> createNativeImageReference() override { return nullptr; }

    void getPixelBuffer(const IntRect&, PixelBuffer&) override { }

    void putPixelBuffer(const PixelBufferSourceView&, const IntRect&, const IntPoint&, AlphaPremultiplication) override { }

    bool canMapBackingStore() const override { return false; }

    unsigned bytesPerRow() const override
    {
        return m_rowbytes > 0 ? static_cast<unsigned>(m_rowbytes) : static_cast<unsigned>(parameters().backendSize.width() * 4);
    }

    String debugDescription() const override { return "ImageBufferWKCBackend"_s; }

    ImageBufferBackendSharing* toBackendSharing() override { return nullptr; }

private:
    void* m_offscreen;
    void* m_dc;
    ImageWKC* m_image;
    int m_rowbytes;
    std::unique_ptr<GraphicsContext> m_graphicsContext;
};

} // namespace WebCore

#endif // !USE(WKC_CAIRO)
