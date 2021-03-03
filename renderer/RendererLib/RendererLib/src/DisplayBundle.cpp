//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayBundle.h"
#include "RendererLib/RenderBackend.h"
#include "RendererAPI/IPlatform.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/IDevice.h"
#include "Watchdog/IThreadAliveNotifier.h"

namespace ramses_internal
{
    DisplayBundle::DisplayBundle(IRendererSceneEventSender& rendererSceneSender, IPlatform& platform, IThreadAliveNotifier& notifier, std::chrono::milliseconds timingReportingPeriod, const String& kpiFilename)
        : m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(platform, m_rendererScenes, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, m_rendererStatistics)
        , m_sceneStateExecutor(m_renderer, rendererSceneSender, m_rendererEventCollector)
        , m_rendererSceneUpdater(platform, m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, notifier)
        , m_sceneControlLogic(m_rendererSceneUpdater)
        , m_rendererCommandExecutor(m_renderer, m_pendingCommands, m_rendererSceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
        , m_sceneReferenceLogic(m_rendererScenes, m_sceneControlLogic, m_rendererSceneUpdater, rendererSceneSender, m_sceneReferenceOwnership)
        , m_timingReportingPeriod{ timingReportingPeriod }
        , m_kpiMonitor(kpiFilename.empty() ? nullptr : new Monitor(kpiFilename))
    {
        m_rendererSceneUpdater.setSceneReferenceLogicHandler(m_sceneReferenceLogic);
    }

    void DisplayBundle::doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime)
    {
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
        finishFrameStatistics(sleepTime);
    }

    void DisplayBundle::pushAndConsumeCommands(RendererCommands& cmds)
    {
        m_pendingCommands.addAndConsumeCommandsFrom(cmds);
    }

    void DisplayBundle::update()
    {
        m_rendererCommandExecutor.executePendingCommands();
        updateSceneControlLogic();
        m_rendererSceneUpdater.updateScenes();
        m_renderer.updateSystemCompositorController();

        m_expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    }

    void DisplayBundle::render()
    {
        m_renderer.doOneRenderLoop();
        m_rendererSceneUpdater.processScreenshotResults();
    }

    void DisplayBundle::collectEvents()
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };

        m_rendererEventCollector.appendAndConsumePendingEvents(m_rendererEvents, m_sceneControlEvents);
        m_sceneReferenceLogic.extractAndSendSceneReferenceEvents(m_sceneControlEvents);
    }

    void DisplayBundle::finishFrameStatistics(std::chrono::microseconds sleepTime)
    {
        const UInt32 drawCalls = static_cast<UInt32>(m_renderer.getProfilerStatistics().getCounterValues(FrameProfilerStatistics::ECounter::DrawCalls)[m_renderer.getProfilerStatistics().getCurrentFrameId()]);
        m_renderer.getStatistics().frameFinished(drawCalls);
        m_renderer.getProfilerStatistics().markFrameFinished(sleepTime);

        UInt32 drawCallCount(0u);
        UInt32 usedGPUMemory(0u);
        if (m_renderer.hasDisplayController())
        {
            auto& device = m_renderer.getDisplayController().getRenderBackend().getDevice();
            drawCallCount += device.getAndResetDrawCallCount();
            usedGPUMemory += device.getTotalGpuMemoryUsageInKB();
        }

        m_renderer.getProfilerStatistics().setCounterValue(FrameProfilerStatistics::ECounter::DrawCalls, drawCallCount);
        m_renderer.getProfilerStatistics().setCounterValue(FrameProfilerStatistics::ECounter::UsedGPUMemory, usedGPUMemory / 1024);

        if (m_kpiMonitor)
        {
            const auto timeNowMs = PlatformTime::GetMillisecondsMonotonic();
            if (timeNowMs > m_lastUpdateTimeStampMilliSec + MonitorUpdateIntervalInMilliSec)
            {
                const auto& stats = m_renderer.getStatistics();
                const GpuMemorySample memorySample(m_rendererSceneUpdater);
                m_renderer.getMemoryStatistics().addMemorySample(memorySample);
                m_kpiMonitor->recordFrameInfo({ PlatformTime::GetMillisecondsAbsolute(), stats.getFps(), stats.getDrawCallsPerFrame(), usedGPUMemory });
                m_lastUpdateTimeStampMilliSec = timeNowMs;
            }
        }
    }

    void DisplayBundle::updateSceneControlLogic()
    {
        InternalSceneStateEvents internalSceneEvents;
        m_rendererEventCollector.dispatchInternalSceneStateEvents(internalSceneEvents);

        for (const auto& evt : internalSceneEvents)
            m_sceneControlLogic.processInternalEvent(evt);

        RendererSceneControlLogic::Events outSceneEvents;
        m_sceneControlLogic.consumeEvents(outSceneEvents);

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

    IEmbeddedCompositingManager& DisplayBundle::getECManager(DisplayHandle display)
    {
        return m_renderer.getDisplayController(display).getEmbeddedCompositingManager();
    }

    IEmbeddedCompositor& DisplayBundle::getEC(DisplayHandle display)
    {
        return m_renderer.getDisplayController(display).getRenderBackend().getEmbeddedCompositor();
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
                m_rendererEventCollector.addFrameTimingReport(m_maxFrameTime, m_sumFrameTimes / m_loopsWithinMeasurePeriod);
                m_maxFrameTime = std::chrono::microseconds{ 0 };
                m_sumFrameTimes = std::chrono::microseconds{ 0 };
                m_loopsWithinMeasurePeriod = 0u;
            }
            m_loopsWithinMeasurePeriod++;
        }
    }
}
