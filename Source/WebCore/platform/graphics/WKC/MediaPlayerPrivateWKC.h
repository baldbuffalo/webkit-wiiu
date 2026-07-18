/*
 * MediaPlayerPrivateWKC.h
 *
 * Modern (2026) WKC media engine. Implements WebCore::MediaPlayerPrivateInterface
 * for progressive <video src>/<audio src> playback, driving the wkcMediaPlayer*Peer
 * C API (declared in wkc/wkcmediapeer.h, implemented by the Wii U platform layer).
 *
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 * Modernized for the homebrew Wii U port.
 */
#pragma once

#if ENABLE(VIDEO) && PLATFORM(WKC)

#include "DestinationColorSpace.h"
#include "MediaPlayerPrivate.h"
#include "PlatformTimeRanges.h"
#include <wtf/RefCounted.h>
#include <wtf/WeakPtr.h>

namespace WebCore {

class MediaPlayerPrivateWKC final
    : public MediaPlayerPrivateInterface
    , public CanMakeWeakPtr<MediaPlayerPrivateWKC>
    , public RefCounted<MediaPlayerPrivateWKC> {
    WTF_MAKE_TZONE_ALLOCATED(MediaPlayerPrivateWKC);
public:
    explicit MediaPlayerPrivateWKC(MediaPlayer&);
    ~MediaPlayerPrivateWKC();

    void ref() const final { RefCounted::ref(); }
    void deref() const final { RefCounted::deref(); }

    static void registerMediaEngine(MediaEngineRegistrar);
    static void getSupportedTypes(HashSet<String>&);
    static MediaPlayer::SupportsType supportsType(const MediaEngineSupportParameters&);

    // Invoked by the peer through the C notify trampoline (see the .cpp).
    void notifyPlayerState(int state);
    void notifyRequestInvalidate(int x, int y, int w, int h);

private:
    // ---- MediaPlayerPrivateInterface ----
    constexpr MediaPlayerType mediaPlayerType() const final { return MediaPlayerType::WKC; }

    void load(const String& url) final;
#if ENABLE(MEDIA_SOURCE)
    void load(const URL&, const LoadOptions&, MediaSourcePrivateClient&) final { }
#endif
#if ENABLE(MEDIA_STREAM)
    void load(MediaStreamPrivate&) final { }
#endif
    void cancelLoad() final;

    void play() final;
    void pause() final;
    bool paused() const final;

    FloatSize naturalSize() const final;
    bool hasVideo() const final;
    bool hasAudio() const final;
    void setPageIsVisible(bool) final;

    MediaTime duration() const final;
    MediaTime currentTime() const final;
    void seekToTarget(const SeekTarget&) final;
    bool seeking() const final;

    void setRate(float) final;
    void setVolume(float) final;

    MediaPlayer::NetworkState networkState() const final { return m_networkState; }
    MediaPlayer::ReadyState readyState() const final { return m_readyState; }

    MediaTime maxTimeSeekable() const final;
    const PlatformTimeRanges& buffered() const final;
    bool didLoadingProgress() const final;

    void setPresentationSize(const IntSize&) final;
    void paint(GraphicsContext&, const FloatRect&) final;
    DestinationColorSpace colorSpace() final { return DestinationColorSpace::SRGB(); }

    ThreadSafeWeakPtr<MediaPlayer> m_player;
    void* m_peer { nullptr };
    MediaPlayer::NetworkState m_networkState { MediaPlayer::NetworkState::Empty };
    MediaPlayer::ReadyState m_readyState { MediaPlayer::ReadyState::HaveNothing };
    IntSize m_size;
    unsigned m_lastBytesLoaded { 0 };
    mutable PlatformTimeRanges m_buffered;
};

} // namespace WebCore

#endif // ENABLE(VIDEO) && PLATFORM(WKC)
