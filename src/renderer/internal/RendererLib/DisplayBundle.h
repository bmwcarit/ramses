//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Renderer.h"
#include "internal/RendererLib/RendererCommandExecutor.h"
#include "internal/RendererLib/SceneStateExecutor.h"
#include "internal/RendererLib/RendererSceneUpdater.h"
#include "internal/RendererLib/RendererSceneControlLogic.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/SceneReferenceOwnership.h"
#include "internal/RendererLib/SceneReferenceLogic.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/Enums/ELoopMode.h"
#include "internal/RendererLib/RendererEventCollector.h"

namespace ramses::internal
{
    class IPlatform;
    class IRendererSceneEventSender;
    class IEmbeddedCompositingManager;
    class IEmbeddedCompositor;
    class IThreadAliveNotifier;

    class IDisplayBundle
    {
    public:
        virtual void doOneLoop(ELoopMode loopMode, std::chrono::microseconds prevFrameSleepTime) = 0;
        virtual void pushAndConsumeCommands(RendererCommands& cmds) = 0;
        virtual void dispatchRendererEvents(RendererEventVector& events) = 0;
        virtual void dispatchSceneControlEvents(RendererEventVector& events) = 0;
        [[nodiscard]] virtual SceneId findMasterSceneForReferencedScene(SceneId refScene) const = 0;
        virtual void enableContext() = 0;
        virtual IEmbeddedCompositingManager& getECManager() = 0;
        virtual IEmbeddedCompositor& getEC() = 0;
        [[nodiscard]] virtual bool hasSystemCompositorController() const = 0;

        virtual std::atomic_int& traceId() = 0;

        virtual ~IDisplayBundle() = default;
    };

    class DisplayBundle final : public IDisplayBundle
    {
    public:
        DisplayBundle(
            DisplayHandle display,
            IRendererSceneEventSender& rendererSceneSender,
            IPlatform& platform,
            IThreadAliveNotifier& notifier,
            std::chrono::milliseconds timingReportingPeriod);

        void doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime) override;

        void pushAndConsumeCommands(RendererCommands& cmds) override;
        void dispatchRendererEvents(RendererEventVector& events) override;
        void dispatchSceneControlEvents(RendererEventVector& events) override;

        [[nodiscard]] SceneId findMasterSceneForReferencedScene(SceneId refScene) const override;
        void enableContext() override;

        // needed for EC tests...
        IEmbeddedCompositingManager& getECManager() override;
        IEmbeddedCompositor& getEC() override;

        // needed for Renderer lifecycle tests...
        [[nodiscard]] bool hasSystemCompositorController() const override;

        // TODO vaclav remove, debugging only
        std::atomic_int& traceId() override { return m_renderer.m_traceId; }

    private:
        void update();
        void render();

        void collectEvents();
        void finishFrameStatistics(std::chrono::microseconds prevFrameSleepTime);
        void updateSceneControlLogic();
        void updateTiming();

        DisplayHandle             m_display;
        FrameTimer                m_frameTimer;
        RendererEventCollector    m_rendererEventCollector;
        RendererScenes            m_rendererScenes;
        SceneExpirationMonitor    m_expirationMonitor;
        RendererStatistics        m_rendererStatistics;
        Renderer                  m_renderer;
        SceneStateExecutor        m_sceneStateExecutor;
        RendererSceneUpdater      m_rendererSceneUpdater;
        RendererSceneControlLogic m_sceneControlLogic;
        RendererCommandExecutor   m_rendererCommandExecutor;
        SceneReferenceOwnership   m_sceneReferenceOwnership;
        SceneReferenceLogic       m_sceneReferenceLogic;

        RendererCommandBuffer m_pendingCommands;
        std::mutex            m_eventsLock;
        RendererEventVector   m_rendererEvents;
        RendererEventVector   m_sceneControlEvents;

        const std::chrono::milliseconds m_timingReportingPeriod{ 0 };
        std::chrono::microseconds m_sumFrameTimes{ 0 };
        std::chrono::microseconds m_maxFrameTime{ 0 };
        size_t m_loopsWithinMeasurePeriod{ 0u };
    };
}
