#include "config.h"
#include "LocalizedStrings.h"

#include "NotImplemented.h"
#include <wkc/wkcmediapeer.h>
#include <wtf/text/WTFString.h>
#include <limits>

extern "C" WKC_PEER_API const unsigned short* wkcSystemGetLanguagePeer(void);
extern "C" WKC_PEER_API const unsigned short* wkcSystemGetButtonLabelSubmitPeer(void);
extern "C" WKC_PEER_API const unsigned short* wkcSystemGetButtonLabelResetPeer(void);
extern "C" WKC_PEER_API const unsigned short* wkcSystemGetButtonLabelFilePeer(void);
extern "C" WKC_PEER_API const unsigned char* wkcSystemGetSystemStringPeer(const unsigned char* in_key);

namespace WebCore {

String localizedString(const char* cstr)
{
    if (!cstr)
        return String();

    String str = String::fromLatin1(cstr);
    if (str == "Submit"_s)
        return String(reinterpret_cast<const char16_t*>(wkcSystemGetButtonLabelSubmitPeer()));
    if (str == "Reset"_s)
        return String(reinterpret_cast<const char16_t*>(wkcSystemGetButtonLabelResetPeer()));
    if (str == "Choose File"_s || str == "Choose Files"_s)
        return String(reinterpret_cast<const char16_t*>(wkcSystemGetButtonLabelFilePeer()));
#if ENABLE(VIDEO)
    if (str == "Live Broadcast"_s)
        return String(reinterpret_cast<const char16_t*>(wkcMediaPlayerGetUIStringPeer(WKC_MEDIA_UISTRING_BROADCAST)));
    if (str == "Loading..."_s)
        return String(reinterpret_cast<const char16_t*>(wkcMediaPlayerGetUIStringPeer(WKC_MEDIA_UISTRING_LOADING)));
#endif
    if (str == "value missing"_s  ||
        str == "type mismatch"_s  ||
        str == "pattern mismatch"_s ||
        str == "too long"_s       ||
        str == "range underflow"_s ||
        str == "range overflow"_s ||
        str == "step mismatch"_s)
        return String::fromUTF8(reinterpret_cast<const char*>(wkcSystemGetSystemStringPeer(reinterpret_cast<const unsigned char*>(cstr))));

    return str;
}

#if ENABLE(VIDEO)
String localizedMediaTimeDescription(float time)
{
    return String(reinterpret_cast<const char16_t*>(wkcMediaPlayerGetUIStringTimePeer(time)));
}
#endif

} // namespace WebCore
