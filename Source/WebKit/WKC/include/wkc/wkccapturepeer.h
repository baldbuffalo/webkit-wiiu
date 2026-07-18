/*
 * wkccapturepeer.h
 *
 * WKC capture peer ABI for the Wii U GamePad camera and microphone. This is the
 * contract between WebCore's MEDIA_STREAM capture backend (getUserMedia) and the
 * Wii U platform layer. The platform integrator implements these functions
 * against wut's camera (UVC) and microphone APIs; WebCore's WKC capture sources
 * call them to enumerate devices and pump frames.
 *
 * Homebrew Wii U port. This header defines the ABI; both halves share it.
 */
#pragma once
#include <wkc/wkcbase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Capture device kinds. */
enum {
    WKC_CAPTURE_KIND_CAMERA     = 0,
    WKC_CAPTURE_KIND_MICROPHONE = 1,
};

/* Result codes. */
enum {
    WKC_CAPTURE_ERROR_OK           = 0,
    WKC_CAPTURE_ERROR_GENERIC      = -1,
    WKC_CAPTURE_ERROR_NODEVICE     = -2,
    WKC_CAPTURE_ERROR_INUSE        = -3,
    WKC_CAPTURE_ERROR_NOTSUPPORTED = -4,
};

/* Pixel format delivered by the camera peer. */
enum {
    WKC_CAPTURE_PIXELFORMAT_NV12    = 0, /* Y plane + interleaved UV (typical UVC/GamePad) */
    WKC_CAPTURE_PIXELFORMAT_YUY2    = 1,
    WKC_CAPTURE_PIXELFORMAT_RGB0888 = 2, /* 32bpp, x8r8g8b8 */
};

/* Description of one capture device. */
struct WKCCaptureDeviceInfo {
    int   fKind;           /* WKC_CAPTURE_KIND_* */
    char  fId[64];         /* stable, opaque device id */
    char  fLabel[128];     /* human-readable label */
};
typedef struct WKCCaptureDeviceInfo WKCCaptureDeviceInfo;

/* Negotiated video format for an opened camera. */
struct WKCCaptureVideoFormat {
    int fWidth;
    int fHeight;
    int fFrameRate;        /* frames per second */
    int fPixelFormat;      /* WKC_CAPTURE_PIXELFORMAT_* */
};
typedef struct WKCCaptureVideoFormat WKCCaptureVideoFormat;

/* Negotiated audio format for an opened microphone. */
struct WKCCaptureAudioFormat {
    int fSampleRate;       /* Hz, e.g. 48000 */
    int fChannels;         /* 1 (mono) / 2 (stereo) */
    int fBitsPerSample;    /* e.g. 16 */
};
typedef struct WKCCaptureAudioFormat WKCCaptureAudioFormat;

/* Lifecycle / enumeration. */
WKC_PEER_API bool wkcCaptureInitializePeer(void);
WKC_PEER_API void wkcCaptureFinalizePeer(void);

/* Fill out_devices (capacity in_max) with the available devices; returns count. */
WKC_PEER_API int  wkcCaptureEnumerateDevicesPeer(WKCCaptureDeviceInfo* out_devices, int in_max);

/* ---- Camera ---------------------------------------------------------------- */
/* Open the camera identified by in_id, negotiating toward io_format (updated in
   place with the granted format). Returns an opaque handle or null. */
WKC_PEER_API void* wkcCaptureCameraOpenPeer(const char* in_id, WKCCaptureVideoFormat* io_format);
WKC_PEER_API void  wkcCaptureCameraClosePeer(void* in_self);
WKC_PEER_API int   wkcCaptureCameraStartPeer(void* in_self);
WKC_PEER_API int   wkcCaptureCameraStopPeer(void* in_self);
/* Pull the most recent frame into in_buffer (size in_buflen). Returns bytes
   written, 0 if no new frame, or a negative WKC_CAPTURE_ERROR_* on failure.
   out_timestamp receives the capture time in seconds. */
WKC_PEER_API int   wkcCaptureCameraReadFramePeer(void* in_self, unsigned char* in_buffer, int in_buflen, double* out_timestamp);

/* ---- Microphone ------------------------------------------------------------ */
WKC_PEER_API void* wkcCaptureMicOpenPeer(const char* in_id, WKCCaptureAudioFormat* io_format);
WKC_PEER_API void  wkcCaptureMicClosePeer(void* in_self);
WKC_PEER_API int   wkcCaptureMicStartPeer(void* in_self);
WKC_PEER_API int   wkcCaptureMicStopPeer(void* in_self);
/* Pull captured PCM into in_buffer (size in_buflen bytes). Returns bytes written,
   0 if none available, or negative WKC_CAPTURE_ERROR_* on failure. */
WKC_PEER_API int   wkcCaptureMicReadSamplesPeer(void* in_self, unsigned char* in_buffer, int in_buflen, double* out_timestamp);

#ifdef __cplusplus
}
#endif
