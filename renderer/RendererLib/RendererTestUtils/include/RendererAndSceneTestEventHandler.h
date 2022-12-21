//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERANDSCENETESTEVENTHANDLER_H
#define RAMSES_RENDERERANDSCENETESTEVENTHANDLER_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "RamsesRendererImpl.h"
#include "RamsesRendererUtils.h"
#include "PlatformAbstraction/PlatformTime.h"
#include <unordered_set>
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
#include <CoreFoundation/CFRunLoop.h>
#endif

namespace ramses
{
    class RendererAndSceneTestEventHandler final : public RendererEventHandlerEmpty, public RendererSceneControlEventHandlerEmpty
    {
    public:
        explicit RendererAndSceneTestEventHandler(RamsesRenderer& renderer, std::chrono::milliseconds timeout = std::chrono::milliseconds{ 20000u })
            : m_renderer(renderer)
            , m_timeout(timeout.count() > 0 ? timeout : std::chrono::hours{ 24 })
        {
        }        
        
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
        {
            m_scenes[sceneId].state = state;
        }

        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersion) override
        {
            m_scenes[sceneId].version = sceneVersion;
        }

        virtual void sceneExpired(sceneId_t sceneId) override
        {
            m_scenes[sceneId].expired = true;
        }

        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override
        {
            m_scenes[sceneId].expired = false;
        }

        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) override
        {
            if (result == ERendererEventResult_OK)
                m_displays.insert(displayId);
        }

        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) override
        {
            if (result == ERendererEventResult_OK)
                m_displays.erase(displayId);
        }

        virtual void offscreenBufferCreated(displayId_t, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            if (result == ERendererEventResult_OK)
                m_offscreenBuffers.insert({ offscreenBufferId, false });
        }

        virtual void offscreenBufferDestroyed(displayId_t, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            if (result == ERendererEventResult_OK)
                m_offscreenBuffers.erase(offscreenBufferId);
        }

        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t, dataConsumerId_t, bool success) override
        {
            if (success)
                m_offscreenBuffers[offscreenBufferId] = true;
        }

        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override
        {
            if (available)
                m_availableStreams.insert(streamId);
            else
                m_availableStreams.erase(streamId);
        }

        bool waitForSceneState(sceneId_t sceneId, RendererSceneState state)
        {
            return waitUntilOrTimeout([&] { return m_scenes.count(sceneId) > 0 && m_scenes.find(sceneId)->second.state == state; });
        }

        bool waitForDisplayCreation(displayId_t displayId)
        {
            return waitUntilOrTimeout([&] { return m_displays.count(displayId) > 0; });
        }

        bool waitForDisplayDestruction(displayId_t displayId)
        {
            return waitUntilOrTimeout([&] { return m_displays.count(displayId) == 0; });
        }

        bool waitForOffscreenBufferCreation(displayBufferId_t offscreenBufferId)
        {
            return waitUntilOrTimeout([&] { return m_offscreenBuffers.count(offscreenBufferId) > 0; });
        }

        bool waitForOffscreenBufferDestruction(displayBufferId_t offscreenBufferId)
        {
            return waitUntilOrTimeout([&] { return m_offscreenBuffers.count(offscreenBufferId) == 0; });
        }

        bool waitForOffscreenBufferLink(displayBufferId_t offscreenBufferId)
        {
            return waitUntilOrTimeout([&] { return m_offscreenBuffers.count(offscreenBufferId) > 0 && m_offscreenBuffers.find(offscreenBufferId)->second; });
        }

        bool waitForFlush(sceneId_t sceneId, sceneVersionTag_t sceneVersion)
        {
            return waitUntilOrTimeout([&] { return m_scenes.count(sceneId) > 0 && m_scenes.find(sceneId)->second.version == sceneVersion; });
        }

        bool checkExpiredState(sceneId_t sceneId)
        {
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
            return m_scenes.count(sceneId) > 0 && m_scenes.find(sceneId)->second.expired;
        }

        void consumePendingEvents()
        {
            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
        }

        bool waitForStreamSurfaceAvailabilityChange(waylandIviSurfaceId_t streamSource, bool available)
        {
            return waitUntilOrTimeout([&] { return (m_availableStreams.count(streamSource) > 0) == available; });
        }

    private:
        bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction)
        {
            const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + m_timeout;
            while (!conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
            {
                if (!m_renderer.impl.isThreaded())
                    m_renderer.doOneLoop();
                
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.005, true);
#else
                std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });  // will give the renderer time to process changes
#endif

                m_renderer.dispatchEvents(*this);
                m_renderer.getSceneControlAPI()->dispatchEvents(*this);
            }

            return conditionFunction();
        }

        std::unordered_set<displayId_t> m_displays;
        std::unordered_set<waylandIviSurfaceId_t> m_availableStreams;
        std::unordered_map<displayBufferId_t, bool> m_offscreenBuffers; // with linked flag

        struct SceneInfo
        {
            RendererSceneState state = RendererSceneState::Unavailable;
            sceneVersionTag_t version = InvalidSceneVersionTag;
            bool expired = false;
        };
        std::unordered_map<sceneId_t, SceneInfo> m_scenes;

        RamsesRenderer& m_renderer;
        std::chrono::milliseconds m_timeout;
    };
}

#endif
