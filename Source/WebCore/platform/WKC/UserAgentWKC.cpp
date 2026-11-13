/*
 * WKC user-agent implementation.
 *
 * Keep the platform identity truthful while using WebKit's Safari-compatible
 * token layout. The Wii U port does not run the PlayStation or desktop platform
 * implementations, so this translation unit provides the WKC definitions.
 */

#include "config.h"
#include "UserAgent.h"

#include <wtf/NeverDestroyed.h>
#include <wtf/text/MakeString.h>

namespace WebCore {

static String standardUserAgentStatic()
{
    // This token layout is deliberately conservative: sites that recognize
    // WebKit can select their Safari/WebKit path without the browser claiming
    // to be Chromium or a desktop operating system.
    static NeverDestroyed<String> userAgent(
        "Mozilla/5.0 (Nintendo Wii U) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15"_s);
    return userAgent;
}

String standardUserAgent(const String& applicationName, const String& applicationVersion)
{
    if (applicationName.isEmpty())
        return standardUserAgentStatic();

    if (applicationVersion.isEmpty())
        return makeString(standardUserAgentStatic(), ' ', applicationName);

    return makeString(standardUserAgentStatic(), ' ', applicationName, '/', applicationVersion);
}

String standardUserAgentForURL(const URL&)
{
    return standardUserAgentStatic();
}

} // namespace WebCore
