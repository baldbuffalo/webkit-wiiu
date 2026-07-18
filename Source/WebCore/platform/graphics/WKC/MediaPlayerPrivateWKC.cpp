/*
 * MediaPlayerPrivateWKC.cpp
 *
 * Modern (2026) WKC media engine implementation. Maps WebCore's
 * MediaPlayerPrivateInterface onto the wkcMediaPlayer*Peer C API. The peer
 * functions are provided by the Wii U platform layer; the ABI they share with
 * this engine is defined in wkc/wkcmedia.h.
 *
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 * Modernized for the homebrew Wii U port.
 */

#include "config.h"
#include "MediaPlayerPrivateWKC.h"

#if ENABLE(VIDEO) && PLATFORM(WKC)

#include "MediaPlayer.h"
#include "NotImplemented.h"
#include <wtf/HashSet.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/TZoneMallocInlines.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

#include <wkc/wkcmediapeer.h>

namespace WebCore {

WTF_MAKE_TZONE_ALLOCATED_IMPL(MediaPlayerPrivateWKC);

// ---- C notify trampolines handed to the peer ----------------------------------
// in_opaque is the MediaPlayerPrivateWKC* passed to wkcMediaPlayerCreatePeer.

static void notifyPlayerStateProc(void* self, int state)
{
    static_cast<MediaPlayerPrivateWKC*>(self)->notifyPlayerState(state);
}
static void notifyRequestInvalidateProc(void* self, int x, int y, int w, int h)
{
    static_cast<MediaPlayerPrivateWKC*>(self)->notifyRequestInvalidate(x, y, w, h);
}
static void notifyAudioDataAvailableProc(void*, float, unsigned int) { }
static bool chromeVisibleProc(void*) { return true; }
static void* createHTMLMediaElementProc(void*) { return nullptr; }
static void destroyHTMLMediaElementProc(void*, void*) { }

// -------------------------------------------------------------------------------

MediaPlayerPrivateWKC::MediaPlayerPrivateWKC(MediaPlayer& player)
    : m_player(player)
{
    static const WKCMediaPlayerCallbacks cb = {
        notifyPlayerStateProc,
        notifyRequestInvalidateProc,
        notifyAudioDataAvailableProc,
        chromeVisibleProc,
        createHTMLMediaElementProc,
        destroyHTMLMediaElementProc,
    };
    // The webkit networking stack is used for streaming; the stream callbacks
    // are optional for progressive playback and left null here.
    m_peer = wkcMediaPlayerCreatePeer(&cb, nullptr, this);
}

MediaPlayerPrivateWKC::~MediaPlayerPrivateWKC()
{
    if (m_peer) {
        wkcMediaPlayerDeletePeer(m_peer);
        m_peer = nullptr;
    }
}

// ---- Type support -------------------------------------------------------------

void MediaPlayerPrivateWKC::getSupportedTypes(HashSet<String>& types)
{
    // The peer is the authority on codecs; advertise the common containers and
    // let supportsType() defer the final answer to the peer per MIME/codec.
    static constexpr ASCIILiteral mimeTypes[] = {
        "audio/mpeg"_s, "audio/mp4"_s, "audio/aac"_s, "audio/wav"_s, "audio/webm"_s, "audio/ogg"_s,
        "video/mp4"_s, "video/webm"_s, "video/ogg"_s, "video/3gpp"_s,
    };
    for (auto& mime : mimeTypes)
        types.add(String { mime });
}

MediaPlayer::SupportsType MediaPlayerPrivateWKC::supportsType(const MediaEngineSupportParameters& parameters)
{
    auto containerType = parameters.type.containerType();
    if (containerType.isEmpty())
        return MediaPlayer::SupportsType::IsNotSupported;

    auto codecs = parameters.type.parameter("codecs"_s);
    int support = wkcMediaPlayerIsSupportedMIMETypePeer(containerType.utf8().data(),
        codecs.isEmpty() ? nullptr : codecs.utf8().data());

    switch (support) {
    case WKC_MEDIA_SUPPORT_SUPPORTED:      return MediaPlayer::SupportsType::IsSupported;
    case WKC_MEDIA_SUPPORT_MAYBESUPPORTED: return MediaPlayer::SupportsType::MayBeSupported;
    default:                               return MediaPlayer::SupportsType::IsNotSupported;
    }
}

// ---- Loading / transport ------------------------------------------------------

void MediaPlayerPrivateWKC::load(const String& url)
{
    if (!m_peer) {
        m_networkState = MediaPlayer::NetworkState::FormatError;
        if (auto player = m_player.get())
            player->networkStateChanged();
        return;
    }
    wkcMediaPlayerLoadPeer(m_peer, url.utf8().data());
}

void MediaPlayerPrivateWKC::cancelLoad()
{
    if (m_peer)
        wkcMediaPlayerCancelLoadPeer(m_peer);
}

void MediaPlayerPrivateWKC::play()
{
    if (m_peer)
        wkcMediaPlayerPlayPeer(m_peer);
}

void MediaPlayerPrivateWKC::pause()
{
    if (m_peer)
        wkcMediaPlayerPausePeer(m_peer);
}

bool MediaPlayerPrivateWKC::paused() const
{
    return m_peer ? wkcMediaPlayerIsPausedPeer(m_peer) : true;
}

// ---- Track / geometry ---------------------------------------------------------

FloatSize MediaPlayerPrivateWKC::naturalSize() const
{
    if (!m_peer)
        return { };
    WKCSize size = { 0, 0 };
    wkcMediaPlayerNaturalSizePeer(m_peer, &size);
    return FloatSize(size.fWidth, size.fHeight);
}

bool MediaPlayerPrivateWKC::hasVideo() const
{
    return m_peer ? wkcMediaPlayerHasVideoPeer(m_peer) : false;
}

bool MediaPlayerPrivateWKC::hasAudio() const
{
    return m_peer ? wkcMediaPlayerHasAudioPeer(m_peer) : false;
}

void MediaPlayerPrivateWKC::setPageIsVisible(bool visible)
{
    if (m_peer)
        wkcMediaPlayerSetVisiblePeer(m_peer, visible);
}

void MediaPlayerPrivateWKC::setPresentationSize(const IntSize& size)
{
    m_size = size;
    if (m_peer) {
        WKCSize wsize = { size.width(), size.height() };
        wkcMediaPlayerSetSizePeer(m_peer, &wsize);
    }
}

// ---- Time / seeking -----------------------------------------------------------

MediaTime MediaPlayerPrivateWKC::duration() const
{
    if (!m_peer)
        return MediaTime::zeroTime();
    float d = wkcMediaPlayerDurationPeer(m_peer);
    if (std::isinf(d))
        return MediaTime::positiveInfiniteTime();
    if (std::isnan(d) || d < 0)
        return MediaTime::zeroTime();
    return MediaTime::createWithDouble(d);
}

MediaTime MediaPlayerPrivateWKC::currentTime() const
{
    if (!m_peer)
        return MediaTime::zeroTime();
    float t = wkcMediaPlayerCurrentTimePeer(m_peer);
    if (std::isnan(t) || t < 0)
        return MediaTime::zeroTime();
    return MediaTime::createWithDouble(t);
}

void MediaPlayerPrivateWKC::seekToTarget(const SeekTarget& target)
{
    if (m_peer)
        wkcMediaPlayerSeekPeer(m_peer, target.time.toFloat());
}

bool MediaPlayerPrivateWKC::seeking() const
{
    return m_peer ? wkcMediaPlayerIsSeekingPeer(m_peer) : false;
}

MediaTime MediaPlayerPrivateWKC::maxTimeSeekable() const
{
    if (!m_peer)
        return MediaTime::zeroTime();
    return MediaTime::createWithDouble(wkcMediaPlayerMaxTimeSeekablePeer(m_peer));
}

// ---- Rate / volume ------------------------------------------------------------

void MediaPlayerPrivateWKC::setRate(float rate)
{
    if (m_peer)
        wkcMediaPlayerSetRatePeer(m_peer, rate);
}

void MediaPlayerPrivateWKC::setVolume(float volume)
{
    if (m_peer)
        wkcMediaPlayerSetVolumePeer(m_peer, volume);
}

// ---- Buffering ----------------------------------------------------------------

const PlatformTimeRanges& MediaPlayerPrivateWKC::buffered() const
{
    m_buffered.clear();
    if (m_peer) {
        MediaTime dur = duration();
        if (dur > MediaTime::zeroTime()) {
            MediaTime seekable = MediaTime::createWithDouble(wkcMediaPlayerMaxTimeSeekablePeer(m_peer));
            if (seekable > MediaTime::zeroTime())
                m_buffered.add(MediaTime::zeroTime(), seekable);
        }
    }
    return m_buffered;
}

bool MediaPlayerPrivateWKC::didLoadingProgress() const
{
    if (!m_peer)
        return false;
    unsigned loaded = wkcMediaPlayerBytesLoadedPeer(m_peer);
    bool progress = loaded != const_cast<MediaPlayerPrivateWKC*>(this)->m_lastBytesLoaded;
    const_cast<MediaPlayerPrivateWKC*>(this)->m_lastBytesLoaded = loaded;
    return progress;
}

// ---- Painting -----------------------------------------------------------------

void MediaPlayerPrivateWKC::paint(GraphicsContext&, const FloatRect&)
{
    // For non-bitmap video sinks the platform composites the frame directly.
    // Bitmap-sink blitting (wkcMediaPlayerLockImagePeer) is a follow-up.
    notImplemented();
}

// ---- Peer -> engine notifications ---------------------------------------------

void MediaPlayerPrivateWKC::notifyPlayerState(int state)
{
    auto player = m_player.get();
    if (!player)
        return;

    switch (state) {
    case WKC_MEDIA_PLAYERSTATE_NETWORKSTATECHANGED: {
        switch (wkcMediaPlayerNetworkStatePeer(m_peer)) {
        case WKC_MEDIA_NETWORKSTATE_IDLE:         m_networkState = MediaPlayer::NetworkState::Idle; break;
        case WKC_MEDIA_NETWORKSTATE_LOADING:      m_networkState = MediaPlayer::NetworkState::Loading; break;
        case WKC_MEDIA_NETWORKSTATE_LOADED:       m_networkState = MediaPlayer::NetworkState::Loaded; break;
        case WKC_MEDIA_NETWORKSTATE_FORMATERROR:  m_networkState = MediaPlayer::NetworkState::FormatError; break;
        case WKC_MEDIA_NETWORKSTATE_NETWORKERROR: m_networkState = MediaPlayer::NetworkState::NetworkError; break;
        case WKC_MEDIA_NETWORKSTATE_DECODEERROR:  m_networkState = MediaPlayer::NetworkState::DecodeError; break;
        }
        player->networkStateChanged();
        break;
    }
    case WKC_MEDIA_PLAYERSTATE_READYSTATECHANGED: {
        switch (wkcMediaPlayerReadyStatePeer(m_peer)) {
        case WKC_MEDIA_READYSTATE_HAVE_NOTHING:     m_readyState = MediaPlayer::ReadyState::HaveNothing; break;
        case WKC_MEDIA_READYSTATE_HAVE_METADATA:    m_readyState = MediaPlayer::ReadyState::HaveMetadata; break;
        case WKC_MEDIA_READYSTATE_HAVE_CURRENTDATA: m_readyState = MediaPlayer::ReadyState::HaveCurrentData; break;
        case WKC_MEDIA_READYSTATE_HAVE_FUTUREDATA:  m_readyState = MediaPlayer::ReadyState::HaveFutureData; break;
        case WKC_MEDIA_READYSTATE_HAVE_ENOUGHDATA:  m_readyState = MediaPlayer::ReadyState::HaveEnoughData; break;
        }
        player->readyStateChanged();
        break;
    }
    case WKC_MEDIA_PLAYERSTATE_VOLUMECHANGED:        player->volumeChanged(player->volume()); break;
    case WKC_MEDIA_PLAYERSTATE_TIMECHANGED:          player->timeChanged(); break;
    case WKC_MEDIA_PLAYERSTATE_SIZECHANGED:          player->sizeChanged(); break;
    case WKC_MEDIA_PLAYERSTATE_RATECHANGED:          player->rateChanged(); break;
    case WKC_MEDIA_PLAYERSTATE_DURATIONCHANGED:      player->durationChanged(); break;
    case WKC_MEDIA_PLAYERSTATE_PLAYBACKSTATECHANGED: player->playbackStateChanged(); break;
    default:
        break;
    }
}

void MediaPlayerPrivateWKC::notifyRequestInvalidate(int, int, int, int)
{
    if (auto player = m_player.get())
        player->repaint();
}

// ---- Registration -------------------------------------------------------------

class MediaPlayerFactoryWKC final : public MediaPlayerFactory {
    WTF_MAKE_TZONE_ALLOCATED(MediaPlayerFactoryWKC);
    WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR(MediaPlayerFactoryWKC);
private:
    MediaPlayerEnums::MediaEngineIdentifier identifier() const final
    {
        return MediaPlayerEnums::MediaEngineIdentifier::WKC;
    }

    Ref<MediaPlayerPrivateInterface> createMediaEnginePlayer(MediaPlayer& player) const final
    {
        return adoptRef(*new MediaPlayerPrivateWKC(player));
    }

    void getSupportedTypes(HashSet<String>& types) const final
    {
        return MediaPlayerPrivateWKC::getSupportedTypes(types);
    }

    MediaPlayer::SupportsType supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters) const final
    {
        return MediaPlayerPrivateWKC::supportsType(parameters);
    }
};

WTF_MAKE_TZONE_ALLOCATED_IMPL(MediaPlayerFactoryWKC);

void MediaPlayerPrivateWKC::registerMediaEngine(MediaEngineRegistrar registrar)
{
    registrar(makeUnique<MediaPlayerFactoryWKC>());
}

} // namespace WebCore

#endif // ENABLE(VIDEO) && PLATFORM(WKC)
