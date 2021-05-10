//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYBUNDLE_H
#define RAMSES_DISPLAYBUNDLE_H

#include "RendererLib/Renderer.h"
#include "RendererLib/RendererCommandExecutor.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/SceneReferenceOwnership.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererEventCollector.h"
#include "Monitoring/Monitor.h"

namespace ramses_internal
{
    class IPlatform;
    class IRendererSceneEventSender;
    class IEmbeddedCompositingManager;
    class IEmbeddedCompositor;
    class IThreadAliveNotifier;

    class IDisplayBundle
    {
    public:
        virtual void doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime) = 0;
        virtual void pushAndConsumeCommands(RendererCommands& cmds) = 0;
        virtual void dispatchRendererEvents(RendererEventVector& events) = 0;
        virtual void dispatchSceneControlEvents(RendererEventVector& events) = 0;
        virtual SceneId findMasterSceneForReferencedScene(SceneId refScene) const = 0;
        virtual void enableContext() = 0;
        virtual IEmbeddedCompositingManager& getECManager() = 0;
        virtual IEmbeddedCompositor& getEC() = 0;
        virtual bool hasSystemCompositorController() const = 0;

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
            std::chrono::milliseconds timingReportingPeriod,
            const String& kpiFilename = {});

        virtual void doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime) override;

        virtual void pushAndConsumeCommands(RendererCommands& cmds) override;
        virtual void dispatchRendererEvents(RendererEventVector& events) override;
        virtual void dispatchSceneControlEvents(RendererEventVector& events) override;

        virtual SceneId findMasterSceneForReferencedScene(SceneId refScene) const override;
        virtual void enableContext() override;

        // needed for EC tests...
        virtual IEmbeddedCompositingManager& getECManager() override;
        virtual IEmbeddedCompositor& getEC() override;

        // needed for Renderer lifecycle tests...
        virtual bool hasSystemCompositorController() const override;

    private:
        void update();
        void render();

        void collectEvents();
        void finishFrameStatistics(std::chrono::microseconds sleepTime);
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

        std::chrono::milliseconds m_timingReportingPeriod{ 0 };
        std::chrono::microseconds m_sumFrameTimes{ 0 };
        std::chrono::microseconds m_maxFrameTime{ 0 };
        size_t m_loopsWithinMeasurePeriod{ 0u };

        // TODO rework KPI monitor
        uint64_t m_lastUpdateTimeStampMilliSec = 0;
        static constexpr uint64_t MonitorUpdateIntervalInMilliSec = 500u;
        std::unique_ptr<Monitor> m_kpiMonitor;
    };
}

#endif
