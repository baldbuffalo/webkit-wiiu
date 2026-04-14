#pragma once

#include_next <WebCore/PopupMenu.h>
#include <wtf/RefCounted.h>
#include "IntRect.h"

namespace WebCore {

class LocalFrameView;

class PopupMenu : public RefCounted<PopupMenu> {
public:
    virtual ~PopupMenu() = default;
    virtual void show(const IntRect&, LocalFrameView&, int selectedIndex) = 0;
    virtual void hide() = 0;
    virtual void updateFromElement() = 0;
    virtual void disconnectClient() = 0;
};

} // namespace WebCore
