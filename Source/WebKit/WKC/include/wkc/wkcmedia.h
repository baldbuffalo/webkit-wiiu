/*
 * wkcmedia.h
 *
 * WKC media peer ABI. This header was a forward-declaration-only stub in the
 * public tree; the concrete struct/constant definitions live only in the
 * proprietary ACCESS WKC SDK. For the homebrew Wii U port we define the peer
 * contract here ourselves (reconstructed from how MediaPlayerPrivateWKC drives
 * the wkcMediaPlayer*Peer API in wkcmediapeer.h). The platform integrator
 * implements the wkcMediaPlayer*Peer functions against the device's media
 * libraries; both sides share the definitions below, so this IS the ABI.
 */
#pragma once
#include <wkc/wkcbase.h>

/* UI string identifiers (wkcMediaPlayerGetUIStringPeer). */
enum {
    WKC_MEDIA_UISTRING_BROADCAST = 0,
    WKC_MEDIA_UISTRING_LOADING   = 1,
};

/* Result codes returned by the wkcMediaPlayer*Peer functions. */
enum {
    WKC_MEDIA_ERROR_OK           = 0,
    WKC_MEDIA_ERROR_GENERIC      = -1,
    WKC_MEDIA_ERROR_NOTSUPPORTED = -2,
    WKC_MEDIA_ERROR_EOF          = -3,
};

/* Network state (wkcMediaPlayerNetworkStatePeer). */
enum {
    WKC_MEDIA_NETWORKSTATE_IDLE         = 0,
    WKC_MEDIA_NETWORKSTATE_LOADING      = 1,
    WKC_MEDIA_NETWORKSTATE_LOADED       = 2,
    WKC_MEDIA_NETWORKSTATE_FORMATERROR  = 3,
    WKC_MEDIA_NETWORKSTATE_NETWORKERROR = 4,
    WKC_MEDIA_NETWORKSTATE_DECODEERROR  = 5,
};

/* Ready state (wkcMediaPlayerReadyStatePeer). */
enum {
    WKC_MEDIA_READYSTATE_HAVE_NOTHING     = 0,
    WKC_MEDIA_READYSTATE_HAVE_METADATA    = 1,
    WKC_MEDIA_READYSTATE_HAVE_CURRENTDATA = 2,
    WKC_MEDIA_READYSTATE_HAVE_FUTUREDATA  = 3,
    WKC_MEDIA_READYSTATE_HAVE_ENOUGHDATA  = 4,
};

/* Movie load type (wkcMediaPlayerMovieLoadTypePeer). */
enum {
    WKC_MEDIA_MOVIELOADTYPE_UNKNOWN      = 0,
    WKC_MEDIA_MOVIELOADTYPE_DOWNLOAD     = 1,
    WKC_MEDIA_MOVIELOADTYPE_STOREDSTREAM = 2,
    WKC_MEDIA_MOVIELOADTYPE_LIVESTREAM   = 3,
};

/* Player state-change notifications delivered through the per-instance
   fNotifyPlayerStateProc callback. */
enum {
    WKC_MEDIA_PLAYERSTATE_NETWORKSTATECHANGED         = 0,
    WKC_MEDIA_PLAYERSTATE_READYSTATECHANGED           = 1,
    WKC_MEDIA_PLAYERSTATE_VOLUMECHANGED               = 2,
    WKC_MEDIA_PLAYERSTATE_TIMECHANGED                 = 3,
    WKC_MEDIA_PLAYERSTATE_SIZECHANGED                 = 4,
    WKC_MEDIA_PLAYERSTATE_RATECHANGED                 = 5,
    WKC_MEDIA_PLAYERSTATE_DURATIONCHANGED             = 6,
    WKC_MEDIA_PLAYERSTATE_PLAYBACKSTATECHANGED        = 7,
    WKC_MEDIA_PLAYERSTATE_SEEKBEGIN                   = 8,
    WKC_MEDIA_PLAYERSTATE_SEEKEND                     = 9,
    WKC_MEDIA_PLAYERSTATE_RESETUSERGESTURERESTRICTION = 10,
    WKC_MEDIA_PLAYERSTATE_VIDEOPLAYEREND              = 11,
};

/* MIME-type support level (wkcMediaPlayerIsSupportedMIMETypePeer). */
enum {
    WKC_MEDIA_SUPPORT_NOTSUPPORTED   = 0,
    WKC_MEDIA_SUPPORT_MAYBESUPPORTED = 1,
    WKC_MEDIA_SUPPORT_SUPPORTED      = 2,
};

/* Video sink types (wkcMediaPlayerVideoSinkTypePeer). */
enum {
    WKC_MEDIA_VIDEOSINKTYPE_BITMAP      = 0,
    WKC_MEDIA_VIDEOSINKTYPE_WINDOW      = 1,
    WKC_MEDIA_VIDEOSINKTYPE_HOLEDWINDOW = 2,
    WKC_MEDIA_VIDEOSINKTYPE_LAYER       = 3,
};

/* Audio sink types (wkcMediaPlayerAudioSinkTypePeer). */
enum {
    WKC_MEDIA_AUDIOSINKTYPE_BINARYSTREAM = 0,
};

/* Per-instance media player callbacks handed to wkcMediaPlayerCreatePeer.
   in_opaque is the value passed as the create() opaque argument. */
struct WKCMediaPlayerCallbacks {
    void  (*fNotifyPlayerStateProc)(void* in_opaque, int in_state);
    void  (*fNotifyRequestInvalidateProc)(void* in_opaque, int in_x, int in_y, int in_w, int in_h);
    void  (*fNotifyAudioDataAvailableProc)(void* in_opaque, float in_timing, unsigned int in_len);
    bool  (*fChromeVisibleProc)(void* in_opaque);
    void* (*fCreateHTMLMediaElementProc)(void* in_opaque);
    void  (*fDestroyHTMLMediaElementProc)(void* in_opaque, void* in_element);
};

/* Global media player procs registered via wkcMediaPlayerCallbackSetPeer.
   Reserved for future use by the platform layer. */
struct WKCMediaPlayerProcs {
    void* fReserved;
};

#ifndef __cplusplus
typedef struct WKCMediaPlayerCallbacks WKCMediaPlayerCallbacks;
typedef struct WKCMediaPlayerProcs WKCMediaPlayerProcs;
#endif
