/*
 * Copyright (C) 2007 Apple Computer, Kevin Ollivier.  All rights reserved.
 * Copyright (c) 2008, Google Inc. All rights reserved.
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
#include "ImageWKC.h"

#include "BitmapImage.h"
#include "GraphicsContext.h"
#include "ImageObserver.h"
#include "NotImplemented.h"
#include <wtf/FastMalloc.h>
#include <wtf/MathExtras.h>

#include <wkc/wkcgpeer.h>

namespace WebCore {

// WKC ImageFrame::FrameComplete value (was enum in old ImageDecoder.h)
static const int kFrameComplete = 3;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static inline void platformRect(const FloatRect& in, WKCFloatRect& out)
{
    out.fPoint.fX     = in.x();
    out.fPoint.fY     = in.y();
    out.fSize.fWidth  = in.width();
    out.fSize.fHeight = in.height();
}

// ---------------------------------------------------------------------------
// ImageWKC
// ---------------------------------------------------------------------------

int
ImageWKC::platformOperator(CompositeOperator op)
{
    switch (op) {
    case CompositeOperator::Clear:           return WKC_COMPOSITEOPERATION_CLEAR;
    case CompositeOperator::Copy:            return WKC_COMPOSITEOPERATION_COPY;
    case CompositeOperator::SourceOver:      return WKC_COMPOSITEOPERATION_SOURCEOVER;
    case CompositeOperator::SourceIn:        return WKC_COMPOSITEOPERATION_SOURCEIN;
    case CompositeOperator::SourceOut:       return WKC_COMPOSITEOPERATION_SOURCEOUT;
    case CompositeOperator::SourceAtop:      return WKC_COMPOSITEOPERATION_SOURCEATOP;
    case CompositeOperator::DestinationOver: return WKC_COMPOSITEOPERATION_DESTINATIONOVER;
    case CompositeOperator::DestinationIn:   return WKC_COMPOSITEOPERATION_DESTINATIONIN;
    case CompositeOperator::DestinationOut:  return WKC_COMPOSITEOPERATION_DESTINATIONOUT;
    case CompositeOperator::DestinationAtop: return WKC_COMPOSITEOPERATION_DESTINATIONATOP;
    case CompositeOperator::XOR:             return WKC_COMPOSITEOPERATION_XOR;
    case CompositeOperator::PlusDarker:      return WKC_COMPOSITEOPERATION_PLUSDARKER;
    case CompositeOperator::PlusLighter:     return WKC_COMPOSITEOPERATION_PLUSLIGHTER;
    default:                                 return 0;
    }
}

static int gInternalFormat = ImageWKC::EColorRGB565;
static bool gReduceTo565IfPossible = false;

void
ImageWKC::setInternalColorFormatARGB8888(bool reduceifpossible)
{
    gInternalFormat = EColorARGB8888;
    gReduceTo565IfPossible = reduceifpossible;
}

void
ImageWKC::setInternalColorFormatRGB565()
{
    gInternalFormat = EColorRGB565;
    gReduceTo565IfPossible = false;
}

ImageWKC::ImageWKC(int type, void* bitmap, int rowbytes, const IntSize& size, bool ownbitmap, bool onstack)
    : m_refcount(1)
    , m_type(type)
    , m_bitmap(bitmap)
    , m_rowbytes(rowbytes)
    , m_size(size)
    , m_ownbitmap(ownbitmap)
    , m_onstack(onstack)
    , m_hasAlpha(false)
    , m_scalex(1)
    , m_scaley(1)
    , m_decodedLines(0)
    , m_allowReduceColor(gReduceTo565IfPossible)
    , m_offscreen(nullptr)
{
    switch (type) {
    case EColorARGB8888:
        m_bpp = 4;
        break;
    case EColorRGB565:
        m_bpp = 2;
        m_allowReduceColor = false;
        break;
    default:
        ASSERT_NOT_REACHED();
        m_bpp = 4;
        break;
    }
}

ImageWKC*
ImageWKC::create(int type, void* bitmap, int rowbytes, const IntSize& size, bool ownbitmap)
{
    return new ImageWKC(type, bitmap, rowbytes, size, ownbitmap);
}

ImageWKC::~ImageWKC()
{
    if (!m_ownbitmap)
        return;
    if (m_bitmap) {
        WTF::fastFree(m_bitmap);
        m_bitmap = nullptr;
    }
#if !USE(WKC_CAIRO)
    if (m_offscreen) {
        delete m_offscreen;
        m_offscreen = nullptr;
    }
#endif
}

void
ImageWKC::ref()
{
    m_refcount++;
}

void
ImageWKC::unref()
{
    m_refcount--;
    if (!m_refcount && !m_onstack)
        delete this;
}

bool
ImageWKC::resize(const IntSize& size)
{
    if (!m_ownbitmap)
        return false;

    int width = size.width();
    if ((m_bpp == 2) && (width & 1))
        width++;

    const int len = width * size.height();
    void* newbitmap = nullptr;

    WTF::TryMallocReturnValue rv = WTF::tryFastMalloc(len * m_bpp);
    if (!rv.getValue(newbitmap))
        return false;

    if (m_bitmap)
        WTF::fastFree(m_bitmap);
    m_bitmap = newbitmap;
    m_rowbytes = width * m_bpp;
    m_size = size;

#if !USE(WKC_CAIRO)
    if (m_offscreen) {
        delete m_offscreen;
        m_offscreen = nullptr;
    }

    WKCSize osize = { m_size.width(), m_size.height() };
    unsigned int max_tile_width  = 2048;
    unsigned int max_tile_height = 2048;
    if ((unsigned)m_size.width()  < max_tile_width)  max_tile_width  = m_size.width();
    if ((unsigned)m_size.height() < max_tile_height) max_tile_height = m_size.height();
    WKCSize tile_size = { (int)max_tile_width, (int)max_tile_height };
    m_offscreen = new ImageTilesWKC(osize, tile_size);
#endif

    zeroFill();
    return true;
}

void
ImageWKC::clear()
{
    if (!m_ownbitmap)
        return;
    if (m_bitmap) {
        WTF::fastFree(m_bitmap);
        m_bitmap = nullptr;
    }
#if !USE(WKC_CAIRO)
    if (m_offscreen) {
        delete m_offscreen;
        m_offscreen = nullptr;
    }
#endif
}

void
ImageWKC::zeroFillFrameRect(const IntRect& r)
{
    if (!m_bitmap)
        return;
    int x0 = WKC_MAX(0, r.x());
    int x1 = WKC_MIN(m_size.width(), r.maxX());
    int len = (x1 - x0) * m_bpp;
    if (len <= 0)
        return;
    int y0 = WKC_MAX(0, r.y());
    int y1 = WKC_MIN(m_size.height(), r.maxY());
    unsigned char* dst = (unsigned char*)m_bitmap + y0 * m_rowbytes + x0 * m_bpp;
    for (int y = y0; y < y1; y++, dst += m_rowbytes)
        memset(dst, 0, len);
}

void
ImageWKC::zeroFill()
{
    const int len = m_rowbytes * m_size.height();
    if (m_bitmap)
        ::memset(m_bitmap, 0, len);

#if !USE(WKC_CAIRO)
    if (m_offscreen) {
        WKCFloatRect idst;
        WKCFloatRect_SetXYWH(&idst, 0, 0, (float)m_size.width(), (float)m_size.height());
        m_offscreen->ClearRect(idst);
    }
#endif
}

void
ImageWKC::notifyStatus(int status)
{
#if !USE(WKC_CAIRO)
    if (status != kFrameComplete)
        return;
    if (!m_bitmap)
        return;

    if (m_offscreen) {
        WKCPeerImage img = { 0 };
        img.fType     = WKC_IMAGETYPE_ARGB8888;
        img.fBitmap   = m_bitmap;
        img.fRowBytes = m_rowbytes;
        img.fMask         = nullptr;
        img.fMaskRowBytes = 0;
        WKCFloatRect_SetXYWH(&img.fSrcRect, 0, 0, (float)m_size.width(), (float)m_size.height());
        WKCFloatSize_Set(&img.fScale,     1.f, 1.f);
        WKCFloatSize_Set(&img.fiScale,    1.f, 1.f);
        WKCFloatPoint_Set(&img.fPhase,    0.f, 0.f);
        WKCFloatSize_Set(&img.fiTransform,1.f, 1.f);
        img.fOffscreen = nullptr;
        WKCFloatRect idst;
        WKCFloatRect_SetXYWH(&idst, 0, 0, (float)m_size.width(), (float)m_size.height());
        m_offscreen->BitBlt(&img, &idst);
    }
#else
    if (!m_allowReduceColor)
        return;
    if (!m_ownbitmap)
        return;
    if (status != kFrameComplete)
        return;
    if (m_hasAlpha)
        return;
    if (m_type != EColorARGB8888)
        return;

    int w = m_size.width();
    if (w & 1)
        w++;

    WTF::TryMallocReturnValue rv = WTF::tryFastMalloc(w * m_size.height() * 2);
    unsigned short* newimg = nullptr;
    if (!rv.getValue(newimg))
        return;

    const unsigned int* src = (const unsigned int*)m_bitmap;
    for (int y = 0; y < m_size.height(); y++) {
        unsigned short* dest = (unsigned short*)newimg + y * w;
        for (int x = 0; x < m_size.width(); x++) {
            const unsigned int v = *src++;
            *dest++ = (unsigned short)(((v >> 8) & 0xf800) | ((v >> 5) & 0x07e0) | ((v >> 3) & 0x1f));
        }
    }

    WTF::fastFree(m_bitmap);
    m_bitmap    = newimg;
    m_rowbytes  = w * 2;
    m_bpp       = 2;
    m_type      = EColorRGB565;
#endif
}

bool
ImageWKC::copyImage(const ImageWKC* other)
{
    m_rowbytes = other->rowbytes();
    m_bpp      = other->bpp();
    m_type     = other->type();

    if (!resize(IntSize(other->size())))
        return false;

    m_hasAlpha        = other->hasAlpha();
    m_scalex          = other->scalex();
    m_scaley          = other->scaley();
    m_allowReduceColor = other->allowReduceColor();

    if (!other->bitmap()) {
        ::memset(m_bitmap, 0, m_rowbytes * m_size.height());
        notifyStatus(kFrameComplete);
        return true;
    }

    ::memcpy(m_bitmap, other->bitmap(), m_rowbytes * m_size.height());
    notifyStatus(kFrameComplete);
    return true;
}

void
ImageWKC::setARGBLine(int xStart, int xEnd, int y, unsigned char* src)
{
    if (!m_bitmap)
        return;
    if (y >= m_size.height())
        return;
    if (xStart >= xEnd)
        return;

    if (xStart < 0)            xStart = 0;
    else if (xStart >= m_size.width()) return;
    if (xEnd < 0)              return;
    else if (xEnd > m_size.width())    xEnd = m_size.width() - 1;
    int len = xEnd - xStart;

    if (m_decodedLines < y + 1)
        m_decodedLines = y + 1;

    unsigned int r, g, b;

    if (m_type == EColorRGB565) {
        unsigned short* d = reinterpret_cast<unsigned short*>((char*)m_bitmap + y * m_rowbytes + xStart * m_bpp);
        while (len--) {
#if CPU(BIG_ENDIAN)
            src++;
            r = *src++;
            g = *src++;
            b = *src++;
#else
            b = *src++;
            g = *src++;
            r = *src++;
            src++;
#endif
            *d++ = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f);
        }
    } else {
        unsigned int* d = reinterpret_cast<unsigned int*>((char*)m_bitmap + y * m_rowbytes + xStart * m_bpp);
        memcpy(d, src, len * 4);
    }
}

void
ImageWKC::setDecodedLines(int y)
{
    if (m_decodedLines < y + 1)
        m_decodedLines = y + 1;
}

void
ImageWKC::setAllowReduceColor(bool flag)
{
    m_allowReduceColor = flag;
    if (!gReduceTo565IfPossible)
        m_allowReduceColor = false;
}

// ---------------------------------------------------------------------------
// BitmapImage::draw — modern WebKit signature
// ---------------------------------------------------------------------------

ImageDrawResult BitmapImage::draw(GraphicsContext& context, const FloatRect& dst, const FloatRect& src, ImagePaintingOptions options)
{
    if (dst.isEmpty() || src.isEmpty())
        return ImageDrawResult::DidNothing;

    startAnimation();

    // TODO: Implement WKC image drawing using NativeImage/ImageWKC with modern API.
    // For now this is a stub — images will not display.

    if (auto* observer = imageObserver())
        observer->didDraw(*this);

    return ImageDrawResult::DidNothing;
}

// ---------------------------------------------------------------------------
// Image::loadPlatformResource
// ---------------------------------------------------------------------------

RefPtr<Image> Image::loadPlatformResource(const char* name)
{
    const unsigned char* bitmap = nullptr;
    unsigned int width = 0, height = 0;
    bitmap = wkcStockImageGetPlatformResourceImagePeer(name, &width, &height);
    if (!bitmap || !width || !height)
        return nullptr;

    // TODO: Wrap bitmap in a proper NativeImage/BitmapImage for modern WebKit.
    return nullptr;
}

// ---------------------------------------------------------------------------
// ImageTilesWKC (non-Cairo tiled rendering)
// ---------------------------------------------------------------------------

#if !USE(WKC_CAIRO)

// Local helpers — convert between WKCFloatRect and FloatRect
static inline void WKCFloatRect_SetFloatRect(WKCFloatRect* dr, const FloatRect& sr)
{
    WKCFloatRect_SetXYWH(dr, sr.x(), sr.y(), sr.width(), sr.height());
}

static inline void FloatRect_SetWKCFloatRect(FloatRect& dr, const WKCFloatRect& sr)
{
    dr.setX(sr.fPoint.fX);
    dr.setY(sr.fPoint.fY);
    dr.setWidth(sr.fSize.fWidth);
    dr.setHeight(sr.fSize.fHeight);
}

// --- ImgTile ---

ImgTile::ImgTile()
    : m_texture(nullptr)
{
}

ImgTile::ImgTile(const WKCSize& size)
    : m_texture(nullptr)
{
    m_texture = wkcTextureNewPeer(&size);
}

ImgTile::~ImgTile()
{
    if (m_texture)
        wkcTextureDeletePeer(m_texture);
}

void ImgTile::Clip(WKCFloatRect&) { }

void ImgTile::ClearRect(WKCFloatRect& r)
{
    wkcTextureClearImagePeer(m_texture, &r);
}

void ImgTile::Blit(const WKCPeerImage* img, WKCFloatRect& dst) const
{
    wkcTextureSetImagePeer(m_texture, (void*)img);
}

void ImgTile::BlitToDC(void* ctx, WKCPeerImage* img, WKCFloatRect& dst, int op) const
{
    img->fTexture = m_texture;
    wkcDrawContextBitBltPeer(ctx, img, &dst, op);
}

void ImgTile::BlitPatternToDC(void* ctx, WKCPeerImage* img, WKCFloatRect& dst, int op) const
{
    img->fTexture = m_texture;
    wkcDrawContextBlitPatternPeer(ctx, img, &dst, op);
}

// --- ImageTilesWKC ---

ImageTilesWKC::ImageTilesWKC(const WKCSize& size, const WKCSize& maxTileSize)
    : m_size(size)
    , m_numColumns(0)
    , m_maxTileSize(maxTileSize)
{
    m_numColumns = ((m_size.fWidth - 1) / m_maxTileSize.fWidth) + 1;
    int numTiles = (((m_size.fHeight - 1) / m_maxTileSize.fHeight) + 1) * m_numColumns;
    for (int i = 0; i < numTiles; i++)
        m_tiles.append(new ImgTile(m_maxTileSize));
}

ImageTilesWKC::~ImageTilesWKC()
{
    for (int i = 0; i < numTiles(); i++)
        delete m_tiles.at(i);
}

FloatRect
ImageTilesWKC::tileRect(int xIndex, int yIndex) const
{
    int x = xIndex * m_maxTileSize.fWidth;
    int y = yIndex * m_maxTileSize.fHeight;
    return FloatRect(x, y,
        WKC_MIN(m_maxTileSize.fWidth,  m_size.fWidth  - x),
        WKC_MIN(m_maxTileSize.fHeight, m_size.fHeight - y));
}

IntRect
ImageTilesWKC::tilesInRect(const FloatRect& rect) const
{
    int leftIndex   = WKC_MAX(0, static_cast<int>(rect.x()) / m_maxTileSize.fWidth);
    int topIndex    = WKC_MAX(0, static_cast<int>(rect.y()) / m_maxTileSize.fHeight);
    int rightIndex  = (static_cast<int>(ceil(rect.x() + rect.width()))  - 1) / m_maxTileSize.fWidth;
    int bottomIndex = (static_cast<int>(ceil(rect.y() + rect.height())) - 1) / m_maxTileSize.fHeight;
    int columns     = WKC_MIN((rightIndex  - leftIndex)  + 1, m_numColumns);
    int rows        = WKC_MIN((bottomIndex - topIndex)   + 1, m_tiles.size() / m_numColumns);
    return IntRect(leftIndex, topIndex, columns, rows);
}

const ImgTile*
ImageTilesWKC::tile(int xIndex, int yIndex) const
{
    if (xIndex >= m_numColumns)
        return nullptr;
    int i = yIndex * m_numColumns + xIndex;
    if (i >= (int)m_tiles.size())
        return nullptr;
    return m_tiles[i];
}

void
ImageTilesWKC::Clip(WKCFloatRect& r)
{
    for (int i = 0; i < numTiles(); i++) {
        FloatRect ftr = tileRect(i / m_numColumns, i % m_numColumns);
        WKCFloatRect tr;
        WKCFloatRect_SetFloatRect(&tr, ftr);
        if (WKCFloatRect_Intersects(&tr, &r)) {
            WKCFloatRect intersection;
            WKCFloatRect_Intersect(&tr, &r, &intersection);
            m_tiles.at(i)->Clip(intersection);
        }
    }
}

void
ImageTilesWKC::ClearRect(const WKCFloatRect& r)
{
    for (int i = 0; i < numTiles(); i++) {
        FloatRect ftr = tileRect(i % m_numColumns, i / m_numColumns);
        WKCFloatRect tr;
        WKCFloatRect_SetFloatRect(&tr, ftr);
        if (WKCFloatRect_Intersects(&tr, &r)) {
            WKCFloatRect intersect;
            WKCFloatRect_Intersect(&tr, &r, &intersect);
            WKCFloatRect_SetXYWH(&intersect, 0, 0, intersect.fSize.fWidth, intersect.fSize.fHeight);
            m_tiles.at(i)->ClearRect(intersect);
        }
    }
}

void
ImageTilesWKC::BitBlt(WKCPeerImage* in_image, const WKCFloatRect* in_destrect)
{
    if (!in_image)
        return;

    FloatRect dst, src;
    FloatRect_SetWKCFloatRect(dst, *in_destrect);
    FloatRect_SetWKCFloatRect(src, in_image->fSrcRect);

    if ((src != dst) || (src.location() != FloatPoint::zero()) || (dst.location() != FloatPoint::zero()))
        return;

    IntRect drawnTiles = tilesInRect(src);

    for (int yIndex = drawnTiles.y(); yIndex < drawnTiles.y() + drawnTiles.height(); ++yIndex) {
        for (int xIndex = drawnTiles.x(); xIndex < drawnTiles.x() + drawnTiles.width(); ++xIndex) {
            FloatRect tile_rect = tileRect(xIndex, yIndex);
            FloatRect intersect = intersection(src, tile_rect);

            WKCFloatRect_SetFloatRect(&in_image->fSrcRect, intersect);
            WKCFloatRect dstRect;
            WKCFloatRect_SetXYWH(&dstRect, 0, 0, tile_rect.width(), tile_rect.height());

            if (const ImgTile* t = tile(xIndex, yIndex))
                t->Blit(in_image, dstRect);
        }
    }

    WKCFloatRect_SetFloatRect(&in_image->fSrcRect, src);
}

void
ImageTilesWKC::BitBltToDC(void* ctx, WKCPeerImage* in_image, const WKCFloatRect& in_destrect, int op)
{
    if (!in_image)
        return;

    FloatRect dst, src;
    FloatRect_SetWKCFloatRect(dst, in_destrect);
    FloatRect_SetWKCFloatRect(src, in_image->fSrcRect);
    void* image_bitmap = in_image->fBitmap;

    IntRect drawnTiles = tilesInRect(src);
    AffineTransform srcToDst = makeMapBetweenRects(FloatRect(FloatPoint(), src.size()), dst);
    srcToDst.translate(-src.x(), -src.y());

    int bpp = 4;
    int type = in_image->fType & WKC_IMAGETYPE_TYPEMASK;
    if (type == WKC_IMAGETYPE_RGAB5515MASK || type == WKC_IMAGETYPE_RGAB5515)
        bpp = 2;

    for (int yIndex = drawnTiles.y(); yIndex < drawnTiles.y() + drawnTiles.height(); ++yIndex) {
        for (int xIndex = drawnTiles.x(); xIndex < drawnTiles.x() + drawnTiles.width(); ++xIndex) {
            FloatRect tile_rect = tileRect(xIndex, yIndex);
            FloatRect intersect = intersection(src, tile_rect);

            FloatRect d = srcToDst.mapRect(intersect);
            WKCFloatRect dstRect;
            WKCFloatRect_SetFloatRect(&dstRect, d);

            WKCFloatRect_SetXYWH(&in_image->fSrcRect,
                intersect.x() - tile_rect.x(), intersect.y() - tile_rect.y(),
                intersect.width(), intersect.height());

            in_image->fBitmap = (unsigned int*)((char*)image_bitmap + (int)tile_rect.x() * bpp + (int)tile_rect.y() * in_image->fRowBytes);

            if (const ImgTile* t = tile(xIndex, yIndex))
                t->BlitToDC(ctx, in_image, dstRect, op);
        }
    }

    WKCFloatRect_SetFloatRect(&in_image->fSrcRect, src);
    in_image->fBitmap = image_bitmap;
}

void
ImageTilesWKC::BitBltPatternToDC(void* ctx, WKCPeerImage* in_image, const WKCFloatRect& in_destrect, int op)
{
    if (!in_image)
        return;

    void* image_bitmap = in_image->fBitmap;
    FloatRect targetRect, imageRect;
    FloatRect_SetWKCFloatRect(targetRect, in_destrect);
    FloatRect_SetWKCFloatRect(imageRect, in_image->fSrcRect);

    if (imageRect.width() < 2048 && imageRect.height() < 2048) {
        WKCFloatRect dstRect;
        WKCFloatRect_SetFloatRect(&dstRect, targetRect);
        if (const ImgTile* t = tile(0, 0))
            t->BlitPatternToDC(ctx, in_image, dstRect, op);
        in_image->fBitmap = image_bitmap;
        return;
    }

    FloatPoint phase(in_image->fPhase.fX, in_image->fPhase.fY);
    phase.setX(phase.x() <= 0 ? -phase.x() : phase.x());
    phase.setY(phase.y() <= 0 ? -phase.y() : phase.y());

    float tWidth  = targetRect.width();
    float tHeight = targetRect.height();
    float iWidth  = imageRect.width();
    float iHeight = imageRect.height();

    IntRect drawnTiles = tilesInRect(imageRect);
    FloatRect srcRect(phase.x(), phase.y(), iWidth - phase.x(), iHeight - phase.y());
    FloatRect destRect(targetRect.x(), targetRect.y(), iWidth - phase.x(), iHeight - phase.y());
    float ypos = targetRect.y();

    while (tHeight > 0) {
        if (srcRect.height() >= targetRect.height()) {
            destRect.setHeight(targetRect.height());
            srcRect.setHeight(targetRect.height());
        }

        while (tWidth > 0) {
            if (srcRect.width() >= targetRect.width()) {
                srcRect.setWidth(targetRect.width());
                destRect.setWidth(targetRect.width());
            }

            AffineTransform srcToDst = makeMapBetweenRects(FloatRect(FloatPoint(), srcRect.size()), destRect);
            srcToDst.translate(-srcRect.x(), -srcRect.y());

            for (int yIndex = drawnTiles.y(); yIndex < drawnTiles.y() + drawnTiles.height(); ++yIndex) {
                for (int xIndex = drawnTiles.x(); xIndex < drawnTiles.x() + drawnTiles.width(); ++xIndex) {
                    FloatRect tile_rect = tileRect(xIndex, yIndex);
                    FloatRect intersect = intersection(srcRect, tile_rect);
                    FloatRect d = srcToDst.mapRect(intersect);
                    WKCFloatRect dstRect;
                    WKCFloatRect_SetFloatRect(&dstRect, d);
                    WKCFloatRect_SetXYWH(&in_image->fSrcRect,
                        intersect.x() - tile_rect.x(), intersect.y() - tile_rect.y(),
                        intersect.width(), intersect.height());
                    in_image->fBitmap = (unsigned int*)((char*)image_bitmap + (int)tile_rect.x() * 4 + (int)tile_rect.y() * in_image->fRowBytes);
                    if (const ImgTile* t = tile(xIndex, yIndex))
                        t->BlitToDC(ctx, in_image, dstRect, op);
                }
            }

            tWidth -= destRect.width();
            if (destRect.width() > tWidth) {
                destRect.setX(destRect.x() + srcRect.width());
                srcRect  = FloatRect(0, srcRect.y(), tWidth, srcRect.height());
                destRect.setWidth(tWidth);
            } else {
                destRect.setX(destRect.x() + srcRect.width());
                srcRect  = FloatRect(0, srcRect.y(), iWidth, srcRect.height());
                destRect.setWidth(srcRect.width());
            }
        }

        tHeight -= destRect.height();
        if (destRect.height() < tHeight) {
            destRect.setY(ypos + srcRect.height());
            ypos += srcRect.height();
            srcRect  = FloatRect(srcRect.x() + phase.x(), imageRect.y(), iWidth - phase.x(), iHeight);
            destRect.setHeight(srcRect.height());
            tWidth   = targetRect.width();
            destRect.setX(targetRect.x());
            destRect.setWidth(imageRect.width() - phase.x());
        } else {
            destRect.setY(ypos + srcRect.height());
            ypos += tHeight;
            srcRect  = FloatRect(srcRect.x() + phase.x(), imageRect.y(), iWidth - phase.x(), tHeight);
            destRect.setHeight(tHeight);
            tWidth   = targetRect.width();
            destRect.setX(targetRect.x());
            destRect.setWidth(imageRect.width() - phase.x());
        }
    }

    WKCFloatRect_SetFloatRect(&in_image->fSrcRect, imageRect);
    in_image->fBitmap = image_bitmap;
}

#endif // !USE(WKC_CAIRO)

} // namespace WebCore
