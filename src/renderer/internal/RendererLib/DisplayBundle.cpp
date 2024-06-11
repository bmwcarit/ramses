//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DisplayBundle.h"
#include "internal/RendererLib/RenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IPlatform.h"
#include "internal/RendererLib/PlatformInterface/IDisplayController.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/Watchdog/IThreadAliveNotifier.h"

namespace ramses::internal
{
    DisplayBundle::DisplayBundle(
        DisplayHandle display,
        IRendererSceneEventSender& rendererSceneSender,
        IPlatform& platform,
        IThreadAliveNotifier& notifier,
        std::chrono::milliseconds timingReportingPeriod,
        EFeatureLevel featureLevel)
        : m_display(display)
        , m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector, m_rendererStatistics)
        , m_renderer(display, platform, m_rendererScenes, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, m_rendererStatistics)
        , m_sceneStateExecutor(m_renderer, rendererSceneSender, m_rendererEventCollector)
        , m_rendererSceneUpdater(display, platform, m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, notifier, featureLevel)
        , m_sceneControlLogic(m_rendererSceneUpdater)
        , m_rendererCommandExecutor(m_renderer, m_pendingCommands, m_rendererSceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
        , m_sceneReferenceLogic(m_rendererScenes, m_sceneControlLogic, m_rendererSceneUpdater, rendererSceneSender, m_sceneReferenceOwnership)
        , m_timingReportingPeriod{ timingReportingPeriod }
    {
        m_rendererSceneUpdater.setSceneReferenceLogicHandler(m_sceneReferenceLogic);
    }

    void DisplayBundle::doOneLoop(ELoopMode loopMode, std::chrono::microseconds prevFrameSleepTime)
    {
        m_renderer.m_traceId = 1000;
        updateTiming();

        switch (loopMode)
        {
        case ELoopMode::UpdateOnly:
            update();
            break;
        case ELoopMode::UpdateAndRender:
            update();
            render();
            break;
        }

        collectEvents();
        m_renderer.m_traceId = 1100;
        finishFrameStatistics(prevFrameSleepTime);
    }

    void DisplayBundle::pushAndConsumeCommands(RendererCommands& cmds)
    {
        m_pendingCommands.addAndConsumeCommandsFrom(cmds);
    }

    void DisplayBundle::update()
    {
        m_rendererCommandExecutor.executePendingCommands();
        m_renderer.m_traceId = 1001;
        updateSceneControlLogic();
        m_rendererSceneUpdater.updateScenes();
        m_renderer.m_traceId = 1002;
        m_renderer.updateSystemCompositorController();

        m_renderer.m_traceId = 1003;
        m_expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    }

    void DisplayBundle::render()
    {
        m_renderer.doOneRenderLoop();
        m_renderer.m_traceId = 1004;
        m_rendererSceneUpdater.processScreenshotResults();
    }

    void DisplayBundle::collectEvents()
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };

        m_renderer.m_traceId = 1005;
        m_rendererEventCollector.appendAndConsumePendingEvents(m_rendererEvents, m_sceneControlEvents);
        m_renderer.m_traceId = 1006;
        m_sceneReferenceLogic.extractAndSendSceneReferenceEvents(m_sceneControlEvents);
    }

    void DisplayBundle::finishFrameStatistics(std::chrono::microseconds prevFrameSleepTime)
    {
        uint32_t drawCalls = 0u;
        if (m_renderer.hasDisplayController())
            drawCalls = m_renderer.getDisplayController().getRenderBackend().getDevice().getAndResetDrawCallCount();

        m_renderer.getStatistics().frameFinished(drawCalls);
        m_renderer.getProfilerStatistics().markFrameFinished(prevFrameSleepTime);
    }

    void DisplayBundle::updateSceneControlLogic()
    {
        InternalSceneStateEvents internalSceneEvents;
        m_rendererEventCollector.dispatchInternalSceneStateEvents(internalSceneEvents);

        m_renderer.m_traceId = 1010;
        for (const auto& evt : internalSceneEvents)
            m_sceneControlLogic.processInternalEvent(evt);

        m_renderer.m_traceId = 1011;
        RendererSceneControlLogic::Events outSceneEvents;
        m_sceneControlLogic.consumeEvents(outSceneEvents);

        m_renderer.m_traceId = 1012;
        for (const auto& evt : outSceneEvents)
            m_rendererEventCollector.addSceneEvent(ERendererEventType::SceneStateChanged, evt.sceneId, evt.state);
    }

    void DisplayBundle::dispatchRendererEvents(RendererEventVector& events)
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };
        events.swap(m_rendererEvents);
        m_rendererEvents.clear();
    }

    void DisplayBundle::dispatchSceneControlEvents(RendererEventVector& events)
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };
        events.swap(m_sceneControlEvents);
        m_sceneControlEvents.clear();
    }

    SceneId DisplayBundle::findMasterSceneForReferencedScene(SceneId refScene) const
    {
        return m_sceneReferenceOwnership.getSceneOwner(refScene);
    }

    void DisplayBundle::enableContext()
    {
        if (m_renderer.hasDisplayController())
            m_renderer.getDisplayController().enableContext();
    }

    IEmbeddedCompositingManager& DisplayBundle::getECManager()
    {
        return m_renderer.getDisplayController().getEmbeddedCompositingManager();
    }

    IEmbeddedCompositor& DisplayBundle::getEC()
    {
        return m_renderer.getDisplayController().getRenderBackend().getEmbeddedCompositor();
    }

    bool DisplayBundle::hasSystemCompositorController() const
    {
        return m_renderer.hasSystemCompositorController();
    }

    void DisplayBundle::updateTiming()
    {
        const auto lastFrameStart = m_frameTimer.getFrameStartTime();
        m_frameTimer.startFrame();

        if (m_timingReportingPeriod.count() > 0)
        {
            const auto now = m_frameTimer.getFrameStartTime();
            const auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameStart);
            m_maxFrameTime = std::max(m_maxFrameTime, frameTime);
            m_sumFrameTimes += frameTime;
            if (m_sumFrameTimes >= m_timingReportingPeriod && m_loopsWithinMeasurePeriod > 0)
            {
                m_rendererEventCollector.addFrameTimingReport(m_display, m_maxFrameTime, m_sumFrameTimes / m_loopsWithinMeasurePeriod);
                m_maxFrameTime = std::chrono::microseconds{ 0 };
                m_sumFrameTimes = std::chrono::microseconds{ 0 };
                m_loopsWithinMeasurePeriod = 0u;
            }
            m_loopsWithinMeasurePeriod++;
        }
    }
}
