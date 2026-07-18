/*
 * RealtimeMediaSourceCenterWKC.cpp
 *
 * WKC MEDIA_STREAM (getUserMedia) capture backend for the Wii U GamePad camera
 * and microphone. Provides the platform capture factories that WebCore's
 * RealtimeMediaSourceCenter calls, backed by the wkcCapture*Peer C API
 * (wkc/wkccapturepeer.h), which the Wii U platform layer implements.
 *
 * First cut: device enumeration + track lifecycle (open/start/stop/close) are
 * wired to the peer. Actual frame/sample delivery into WebCore VideoFrame /
 * audio buffers is the follow-up (marked TODO below).
 *
 * Homebrew Wii U port.
 */

#include "config.h"

#if ENABLE(MEDIA_STREAM) && PLATFORM(WKC)

#include "CaptureDevice.h"
#include "CaptureDeviceManager.h"
#include "MediaConstraints.h"
#include "RealtimeMediaSource.h"
#include "RealtimeMediaSourceCapabilities.h"
#include "RealtimeMediaSourceCenter.h"
#include "RealtimeMediaSourceFactory.h"
#include "RealtimeMediaSourceSettings.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/ThreadSafeWeakPtr.h>
#include <wtf/text/MakeString.h>

#include <wkc/wkccapturepeer.h>

namespace WebCore {

// ---- Device enumeration -------------------------------------------------------

static const Vector<CaptureDevice>& wkcCaptureDevices()
{
    static NeverDestroyed<Vector<CaptureDevice>> devices = [] {
        Vector<CaptureDevice> list;
        wkcCaptureInitializePeer();
        WKCCaptureDeviceInfo infos[8];
        int count = wkcCaptureEnumerateDevicesPeer(infos, 8);
        for (int i = 0; i < count; ++i) {
            auto type = infos[i].fKind == WKC_CAPTURE_KIND_CAMERA
                ? CaptureDevice::DeviceType::Camera
                : CaptureDevice::DeviceType::Microphone;
            CaptureDevice device(String::fromUTF8(infos[i].fId), type, String::fromUTF8(infos[i].fLabel));
            device.setEnabled(true);
            list.append(WTFMove(device));
        }
        return list;
    }();
    return devices;
}

class WKCCaptureDeviceManager final : public CaptureDeviceManager {
public:
    static WKCCaptureDeviceManager& singleton()
    {
        static NeverDestroyed<WKCCaptureDeviceManager> manager;
        return manager.get();
    }

    const Vector<CaptureDevice>& captureDevices() final { return wkcCaptureDevices(); }
};

// ---- Capture sources ----------------------------------------------------------
//
// Both camera and microphone are modelled as plain RealtimeMediaSource
// subclasses. capabilities()/settings() advertise the negotiated format;
// startProducingData()/stopProducingData() drive the peer. Frame delivery is a
// follow-up: the peer read functions (wkcCaptureCameraReadFramePeer /
// wkcCaptureMicReadSamplesPeer) feed WebCore VideoFrame/audio buffers.

class WKCRealtimeVideoSource final : public RealtimeMediaSource,
    public ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<WKCRealtimeVideoSource, WTF::DestructionThread::MainRunLoop> {
public:
    WTF_ABSTRACT_THREAD_SAFE_REF_COUNTED_AND_CAN_MAKE_WEAK_PTR_IMPL;

    static CaptureSourceOrError create(const CaptureDevice& device, MediaDeviceHashSalts&& salts, std::optional<PageIdentifier> page)
    {
        WKCCaptureVideoFormat fmt = { 640, 480, 30, WKC_CAPTURE_PIXELFORMAT_NV12 };
        void* peer = wkcCaptureCameraOpenPeer(device.persistentId().utf8().data(), &fmt);
        if (!peer)
            return CaptureSourceOrError({ "Failed to open GamePad camera"_s, MediaAccessDenialReason::HardwareError });
        return CaptureSourceOrError(adoptRef(*new WKCRealtimeVideoSource(device, WTFMove(salts), page, peer, fmt)));
    }

    ~WKCRealtimeVideoSource()
    {
        if (m_peer)
            wkcCaptureCameraClosePeer(m_peer);
    }

private:
    WKCRealtimeVideoSource(const CaptureDevice& device, MediaDeviceHashSalts&& salts, std::optional<PageIdentifier> page, void* peer, const WKCCaptureVideoFormat& fmt)
        : RealtimeMediaSource(device, WTFMove(salts), page)
        , m_peer(peer)
        , m_format(fmt)
    {
    }

    void startProducingData() final { if (m_peer) wkcCaptureCameraStartPeer(m_peer); }
    void stopProducingData() final { if (m_peer) wkcCaptureCameraStopPeer(m_peer); }

    const RealtimeMediaSourceSettings& settings() final
    {
        if (!m_settings) {
            RealtimeMediaSourceSettings settings;
            settings.setWidth(m_format.fWidth);
            settings.setHeight(m_format.fHeight);
            settings.setFrameRate(m_format.fFrameRate);
            RealtimeMediaSourceSupportedConstraints constraints;
            constraints.setSupportsWidth(true);
            constraints.setSupportsHeight(true);
            constraints.setSupportsFrameRate(true);
            constraints.setSupportsDeviceId(true);
            settings.setSupportedConstraints(constraints);
            m_settings = WTFMove(settings);
        }
        return *m_settings;
    }

    const RealtimeMediaSourceCapabilities& capabilities() final
    {
        if (!m_capabilities) {
            RealtimeMediaSourceCapabilities capabilities(settings().supportedConstraints());
            capabilities.setWidth({ 1, m_format.fWidth });
            capabilities.setHeight({ 1, m_format.fHeight });
            capabilities.setFrameRate({ 1, static_cast<double>(m_format.fFrameRate) });
            capabilities.setDeviceId(hashedId());
            m_capabilities = WTFMove(capabilities);
        }
        return *m_capabilities;
    }

    void* m_peer { nullptr };
    WKCCaptureVideoFormat m_format;
    std::optional<RealtimeMediaSourceSettings> m_settings;
    std::optional<RealtimeMediaSourceCapabilities> m_capabilities;
};

class WKCRealtimeAudioSource final : public RealtimeMediaSource,
    public ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<WKCRealtimeAudioSource, WTF::DestructionThread::MainRunLoop> {
public:
    WTF_ABSTRACT_THREAD_SAFE_REF_COUNTED_AND_CAN_MAKE_WEAK_PTR_IMPL;

    static CaptureSourceOrError create(const CaptureDevice& device, MediaDeviceHashSalts&& salts, std::optional<PageIdentifier> page)
    {
        WKCCaptureAudioFormat fmt = { 48000, 1, 16 };
        void* peer = wkcCaptureMicOpenPeer(device.persistentId().utf8().data(), &fmt);
        if (!peer)
            return CaptureSourceOrError({ "Failed to open GamePad microphone"_s, MediaAccessDenialReason::HardwareError });
        return CaptureSourceOrError(adoptRef(*new WKCRealtimeAudioSource(device, WTFMove(salts), page, peer, fmt)));
    }

    ~WKCRealtimeAudioSource()
    {
        if (m_peer)
            wkcCaptureMicClosePeer(m_peer);
    }

private:
    WKCRealtimeAudioSource(const CaptureDevice& device, MediaDeviceHashSalts&& salts, std::optional<PageIdentifier> page, void* peer, const WKCCaptureAudioFormat& fmt)
        : RealtimeMediaSource(device, WTFMove(salts), page)
        , m_peer(peer)
        , m_format(fmt)
    {
    }

    void startProducingData() final { if (m_peer) wkcCaptureMicStartPeer(m_peer); }
    void stopProducingData() final { if (m_peer) wkcCaptureMicStopPeer(m_peer); }

    const RealtimeMediaSourceSettings& settings() final
    {
        if (!m_settings) {
            RealtimeMediaSourceSettings settings;
            settings.setSampleRate(m_format.fSampleRate);
            RealtimeMediaSourceSupportedConstraints constraints;
            constraints.setSupportsSampleRate(true);
            constraints.setSupportsDeviceId(true);
            settings.setSupportedConstraints(constraints);
            m_settings = WTFMove(settings);
        }
        return *m_settings;
    }

    const RealtimeMediaSourceCapabilities& capabilities() final
    {
        if (!m_capabilities) {
            RealtimeMediaSourceCapabilities capabilities(settings().supportedConstraints());
            capabilities.setSampleRate({ 1, m_format.fSampleRate });
            capabilities.setDeviceId(hashedId());
            m_capabilities = WTFMove(capabilities);
        }
        return *m_capabilities;
    }

    void* m_peer { nullptr };
    WKCCaptureAudioFormat m_format;
    std::optional<RealtimeMediaSourceSettings> m_settings;
    std::optional<RealtimeMediaSourceCapabilities> m_capabilities;
};

// ---- Factories ----------------------------------------------------------------

class WKCVideoCaptureFactory final : public VideoCaptureFactory {
public:
    CaptureSourceOrError createVideoCaptureSource(const CaptureDevice& device, MediaDeviceHashSalts&& salts, const MediaConstraints*, std::optional<PageIdentifier> page) final
    {
        return WKCRealtimeVideoSource::create(device, WTFMove(salts), page);
    }
    CaptureDeviceManager& videoCaptureDeviceManager() final { return WKCCaptureDeviceManager::singleton(); }
};

class WKCAudioCaptureFactory final : public AudioCaptureFactory {
public:
    CaptureSourceOrError createAudioCaptureSource(const CaptureDevice& device, MediaDeviceHashSalts&& salts, const MediaConstraints*, std::optional<PageIdentifier> page) final
    {
        return WKCRealtimeAudioSource::create(device, WTFMove(salts), page);
    }
    CaptureDeviceManager& audioCaptureDeviceManager() final { return WKCCaptureDeviceManager::singleton(); }
    const Vector<CaptureDevice>& speakerDevices() const final
    {
        static NeverDestroyed<Vector<CaptureDevice>> none;
        return none.get();
    }
};

// ---- Platform seams called by RealtimeMediaSourceCenter -----------------------

AudioCaptureFactory& RealtimeMediaSourceCenter::defaultAudioCaptureFactory()
{
    static NeverDestroyed<WKCAudioCaptureFactory> factory;
    return factory.get();
}

VideoCaptureFactory& RealtimeMediaSourceCenter::defaultVideoCaptureFactory()
{
    static NeverDestroyed<WKCVideoCaptureFactory> factory;
    return factory.get();
}

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM) && PLATFORM(WKC)
