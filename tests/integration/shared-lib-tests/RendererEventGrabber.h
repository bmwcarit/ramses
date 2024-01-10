//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"

#include <unordered_set>
#include <thread>
#include <chrono>

namespace ramses::internal
{
    class RendererEventGrabber : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
    {
    public:
        explicit RendererEventGrabber(RamsesRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        void dispatchEvents()
        {
            m_renderer->dispatchEvents(*this);
            m_renderer->getSceneControlAPI()->dispatchEvents(*this);
        }

        void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult::Failed != result)
            {
                m_displays.insert(displayId);
            }
            else
            {
                m_isRunning = false;
            }
        }

        void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            if (ramses::ERendererEventResult::Failed != result)
            {
                m_displays.erase(displayId);
            }
            else
            {
                m_isRunning = false;
            }
        }

        [[nodiscard]] bool waitForDisplay(ramses::displayId_t displayId)
        {
            return waitUntil([&] { return m_displays.find(displayId) != m_displays.end(); });
        }

        [[nodiscard]] bool waitUntil(const std::function<bool()>& conditionFunction)
        {
            const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{5};
            while (m_isRunning && !conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{5}); // will give the renderer time to process changes
                m_renderer->dispatchEvents(*this);
                auto* sceneControl = m_renderer->getSceneControlAPI();
                sceneControl->dispatchEvents(*this);
            }

            return conditionFunction();
        }

    private:
        using DisplaySet = std::unordered_set<ramses::displayId_t>;

        RamsesRenderer* m_renderer;
        DisplaySet m_displays;
        bool m_isRunning = true;
    };
}
