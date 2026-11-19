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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE COMPUTER, INC. OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(GAMEPAD)

#include "GamepadProvider.h"
#include "GamepadProviderClient.h"
#include "PlatformGamepad.h"
#include "SharedGamepadValue.h"
#include <wtf/MonotonicTime.h>
#include <wtf/NeverDestroyed.h>
#include <memory>

namespace WebCore {

class WKCPlatformGamepad final : public PlatformGamepad {
public:
    explicit WKCPlatformGamepad(unsigned index)
        : PlatformGamepad(index)
    {
        m_mapping = "standard"_s;
    }

    const Vector<SharedGamepadValue>& axisValues() const override { return m_axes; }
    const Vector<SharedGamepadValue>& buttonValues() const override { return m_buttons; }

    void updateId(const String& id) { m_id = id; }
    void updateTimestamp(MonotonicTime timestamp) { m_lastUpdateTime = timestamp; }

    void updateAxes(int count, const float* values)
    {
        m_axes.clear();
        if (!values)
            return;
        for (int i = 0; i < count; ++i)
            m_axes.append(SharedGamepadValue(values[i]));
    }

    void updateButtons(int count, const float* values)
    {
        m_buttons.clear();
        if (!values)
            return;
        for (int i = 0; i < count; ++i)
            m_buttons.append(SharedGamepadValue(values[i]));
    }

private:
    Vector<SharedGamepadValue> m_axes;
    Vector<SharedGamepadValue> m_buttons;
};

class WKCGamepadProvider final : public GamepadProvider {
public:
    static WKCGamepadProvider& singleton()
    {
        static NeverDestroyed<WKCGamepadProvider> provider;
        return provider;
    }

    void startMonitoringGamepads(GamepadProviderClient& client) override
    {
        m_clients.add(client);
        for (auto& gamepad : m_gamepads)
            client.platformGamepadConnected(*gamepad, EventMakesGamepadsVisible::No);
    }

    void stopMonitoringGamepads(GamepadProviderClient& client) override
    {
        m_clients.remove(client);
    }

    const Vector<WeakPtr<PlatformGamepad>>& platformGamepads() override
    {
        return m_gamepadPointers;
    }

    void playEffect(unsigned, const String&, GamepadHapticEffectType,
        const GamepadEffectParameters&, CompletionHandler<void(bool)>&& completionHandler) override
    {
        completionHandler(false);
    }

    void stopEffects(unsigned, const String&, CompletionHandler<void()>&& completionHandler) override
    {
        completionHandler();
    }

    void initializeGamepads(int count)
    {
        m_gamepads.clear();
        m_gamepadPointers.clear();
        for (int i = 0; i < count; ++i) {
            auto gamepad = std::make_unique<WKCPlatformGamepad>(static_cast<unsigned>(i));
            m_gamepadPointers.append(gamepad.get());
            m_gamepads.append(WTF::move(gamepad));
        }
    }

    bool notifyGamepadEvent(int index, const String& id, MonotonicTime timestamp,
        int axisCount, const float* axes, int buttonCount, const float* buttons)
    {
        if (index < 0 || index >= static_cast<int>(m_gamepads.size()))
            return false;

        auto& gamepad = *m_gamepads[index];
        gamepad.updateId(id);
        gamepad.updateTimestamp(timestamp);
        gamepad.updateAxes(axisCount, axes);
        gamepad.updateButtons(buttonCount, buttons);

        setShouldMakeGamepadsVisibile();
        dispatchPlatformGamepadInputActivity();
        return true;
    }

private:
    Vector<std::unique_ptr<WKCPlatformGamepad>> m_gamepads;
    Vector<WeakPtr<PlatformGamepad>> m_gamepadPointers;
};

void initializeGamepads(int pads)
{
    auto& provider = WKCGamepadProvider::singleton();
    GamepadProvider::setSharedProvider(provider);
    provider.initializeGamepads(pads);
}

bool notifyGamepadEvent(int index, const String& id, long long timestamp,
    int axisCount, const float* axes, int buttonCount, const float* buttons)
{
    auto& provider = WKCGamepadProvider::singleton();
    GamepadProvider::setSharedProvider(provider);
    return provider.notifyGamepadEvent(index, id,
        MonotonicTime::fromRawSeconds(timestamp / 1000.0),
        axisCount, axes, buttonCount, buttons);
}

} // namespace WebCore

#endif // ENABLE(GAMEPAD)
