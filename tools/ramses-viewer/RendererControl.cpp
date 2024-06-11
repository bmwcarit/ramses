//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererControl.h"
#include <thread>
#include "impl/RendererEventChainer.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"

namespace ramses::internal
{
    RendererControl::RendererControl(RamsesRenderer* renderer, displayId_t display, uint32_t displayWidth, uint32_t displayHeight,
                                     IRendererEventHandler* rendererEventHandler, IRendererSceneControlEventHandler* sceneControlEventHandler)
        : m_renderer(renderer)
        , m_rendererEventHandler(rendererEventHandler)
        , m_sceneControlEventHandler(sceneControlEventHandler)
        , m_displayId(display)
        , m_displayWidth(displayWidth)
        , m_displayHeight(displayHeight)
    {
    }

    void RendererControl::setupSceneState(sceneId_t sceneId, RendererSceneState state, displayBufferId_t buffer, int32_t renderOrder)
    {
        m_scenes[sceneId].configuredManually = true;
        m_scenes[sceneId].buffer = buffer;
        m_scenes[sceneId].renderOrder = renderOrder;
        auto* sceneControl = m_renderer->getSceneControlAPI();
        if (m_scenes[sceneId].state <= RendererSceneState::Available)
            sceneControl->setSceneMapping(sceneId, m_displayId);
        sceneControl->setSceneDisplayBufferAssignment(sceneId, buffer, renderOrder);
        sceneControl->setSceneState(sceneId, state);
        sceneControl->flush();
    }

    void RendererControl::setupSceneState(sceneId_t sceneId, RendererSceneState state, int32_t renderOrder)
    {
        setupSceneState(sceneId, state, m_scenes[sceneId].buffer, renderOrder);
    }

    void RendererControl::dispatchEvents()
    {
        if (m_rendererEventHandler)
        {
            RendererEventChainer chainer2{ *this, *m_rendererEventHandler };
            m_renderer->dispatchEvents(chainer2);
        }
        else
        {
            m_renderer->dispatchEvents(*this);
        }
        if (m_sceneControlEventHandler)
        {
            RendererSceneControlEventChainer chainer{ *this, *m_sceneControlEventHandler };
            m_renderer->getSceneControlAPI()->dispatchEvents(chainer);
        }
        else
        {
            m_renderer->getSceneControlAPI()->dispatchEvents(*this);
        }
        m_renderer->flush();
        m_renderer->getSceneControlAPI()->flush();
    }

    void RendererControl::sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        const bool scenePublished = m_scenes[sceneId].state == RendererSceneState::Unavailable;
        m_scenes[sceneId].state = state;

        if (m_autoShow && scenePublished && state == RendererSceneState::Available && !m_scenes[sceneId].configuredManually)
        {
            auto* sceneControl = m_renderer->getSceneControlAPI();
            sceneControl->setSceneMapping(sceneId, m_displayId);
            sceneControl->setSceneState(sceneId, RendererSceneState::Rendered);
        }
    }

    void RendererControl::sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion)
    {
        m_scenes[sceneId].version = sceneVersion;
    }

    void RendererControl::offscreenBufferCreated(ramses::displayId_t /*displayId_t*/, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult::Failed != result)
        {
            m_offscreenBuffers.insert(offscreenBufferId);
        }
    }

    void RendererControl::offscreenBufferLinked(ramses::displayBufferId_t /*offscreenBufferId*/, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t /*consumerId*/, bool success)
    {
        if (success)
        {
            m_scenesConsumingOffscreenBuffer[consumerScene].state = ramses::RendererSceneState::Unavailable;
        }
    }

    void RendererControl::displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
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

    void RendererControl::displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
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

    void RendererControl::windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height)
    {
        if (m_displayId == displayId)
        {
            m_displayWidth  = width;
            m_displayHeight = height;
        }
    }

    void RendererControl::windowClosed(ramses::displayId_t /*displayId*/)
    {
        m_isRunning = false;
    }

    void RendererControl::framebufferPixelsRead(const uint8_t* pixelData,
                               const uint32_t pixelDataSize,
                               ramses::displayId_t displayId,
                               ramses::displayBufferId_t displayBuffer,
                               ramses::ERendererEventResult result)
    {
        static_cast<void>(displayId);
        static_cast<void>(displayBuffer);
        if (!m_screenshot.empty())
        {
            m_screenshotSaved = false;
            if (result == ramses::ERendererEventResult::Ok)
            {
                std::vector<uint8_t> buffer;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                buffer.insert(buffer.end(), &pixelData[0], &pixelData[pixelDataSize]);
                m_screenshotSaved = ramses::RamsesUtils::SaveImageBufferToPng(m_screenshot, buffer, m_screenshotWidth, m_screenshotHeight, true);
            }
            m_screenshot.clear();
        }
    }

    bool RendererControl::waitForDisplay(ramses::displayId_t displayId)
    {
        return waitUntil([&] { return m_displays.find(displayId) != m_displays.end(); });
    }

    bool RendererControl:: waitForSceneState(ramses::Scene& scene, ramses::RendererSceneState state)
    {
        return waitUntil([&] { return m_scenes[scene.getSceneId()].state == state; }, [&]() { scene.flush(); });
    }

    bool RendererControl::waitForSceneVersion(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t version)
    {
        return waitUntil([&] { return m_scenes[sceneId].version == version; });
    }

    bool RendererControl::waitForOffscreenBufferCreated(const ramses::displayBufferId_t offscreenBufferId)
    {
        return waitUntil([&] { return m_offscreenBuffers.find(offscreenBufferId) != m_offscreenBuffers.end(); });
    }

    bool RendererControl::waitForOffscreenBufferLinked(const ramses::sceneId_t sceneId)
    {
        return waitUntil([&] { return m_scenesConsumingOffscreenBuffer.count(sceneId) > 0; });
    }

    bool RendererControl::waitForScreenshot()
    {
        waitUntil([&] { return m_screenshot.empty(); });
        return m_screenshotSaved;
    }

    bool RendererControl::waitUntil(const std::function<bool()>& conditionFunction, const std::function<void()>& dispatch)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{5};
        while (m_isRunning && !conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            if (dispatch)
                dispatch();
            std::this_thread::sleep_for(std::chrono::milliseconds{5}); // will give the renderer time to process changes
            dispatchEvents();
        }

        return conditionFunction();
    }

    bool RendererControl::saveScreenshot(const std::string& filename)
    {
        return saveScreenshot(filename, ramses::displayBufferId_t(), 0, 0, m_displayWidth, m_displayHeight);
    }

    bool RendererControl::saveScreenshot(const std::string& filename, ramses::displayBufferId_t screenshotBuf, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        if (m_renderer && m_screenshot.empty() && !filename.empty())
        {
            m_screenshotSaved = false;
            m_screenshot = filename;
            m_screenshotWidth = width - x;
            m_screenshotHeight = height - x;
            m_renderer->readPixels(m_displayId, screenshotBuf, x, y, width, height);
            m_renderer->flush();
            return true;
        }
        return false;
    }

}
