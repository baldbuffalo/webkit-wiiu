/*
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
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

#if ENABLE(GAMEPAD)

#include "GamepadProvider.h"
#include "GamepadProviderClient.h"
#include "PlatformGamepad.h"
#include <wtf/MonotonicTime.h>
#include <wtf/NeverDestroyed.h>

namespace WebCore {

class WKCGamepadProvider : public GamepadProvider {
public:
    static WKCGamepadProvider& singleton()
    {
        static NeverDestroyed<WKCGamepadProvider> provider;
        return provider;
    }

    void startMonitoringGamepads(GamepadProviderClient&) override { }
    void stopMonitoringGamepads(GamepadProviderClient&) override { }

    const Vector<WeakPtr<PlatformGamepad>>& platformGamepads() override
    {
        return m_gamepadPtrs;
    }

    void initializeGamepads(int count)
    {
        m_gamepads.clear();
        m_gamepadPtrs.clear();
        for (int i = 0; i < count; i++) {
            auto pad = makeUnique<PlatformGamepad>(i);
            m_gamepadPtrs.append(pad.get());
            m_gamepads.append(WTFMove(pad));
        }
    }

    bool notifyGamepadEvent(int index, const String& id, MonotonicTime timestamp,
                            int naxes, const float* axes, int nbuttons, const float* buttons)
    {
        if (index >= (int)m_gamepads.size())
            return false;

        auto& pad = *m_gamepads[index];
        pad.updateId(id);
        pad.updateTimestamp(timestamp);

        if (naxes && axes) {
            Vector<double> axisValues(naxes);
            for (int i = 0; i < naxes; i++)
                axisValues[i] = axes[i];
            pad.updateAxes(axisValues);
        }

        if (nbuttons && buttons) {
            Vector<double> buttonValues(nbuttons);
            for (int i = 0; i < nbuttons; i++)
                buttonValues[i] = buttons[i];
            pad.updateButtons(buttonValues);
        }

        for (auto& client : m_clients)
            client->platformGamepadInputActivity(EventMakesGamepadVisible::Yes);

        return true;
    }

private:
    Vector<UniquePtr<PlatformGamepad>> m_gamepads;
    Vector<WeakPtr<PlatformGamepad>>  m_gamepadPtrs;
};

void initializeGamepads(int pads)
{
    WKCGamepadProvider::singleton().initializeGamepads(pads);
}

bool notifyGamepadEvent(int index, const String& id, long long timestamp,
                        int naxes, const float* axes, int nbuttons, const float* buttons)
{
    return WKCGamepadProvider::singleton().notifyGamepadEvent(
        index, id,
        MonotonicTime::fromRawSeconds(timestamp / 1000.0),
        naxes, axes, nbuttons, buttons);
}

} // namespace WebCore

#endif // ENABLE(GAMEPAD)
