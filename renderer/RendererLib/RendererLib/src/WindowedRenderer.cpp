//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/WindowedRenderer.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/FrameTimer.h"

#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IPlatformFactory.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IWindow.h"

#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/LogMacros.h"
#include "RendererLib/RendererCachedScene.h"
#include "Ramsh/Ramsh.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    WindowedRenderer::WindowedRenderer(
        RendererCommandBuffer& commandBuffer,
        IRendererSceneEventSender& rendererSceneSender,
        IPlatformFactory& platformFactory,
        RendererStatistics& rendererStatistics,
        const String& monitorFilename)
        : m_rendererCommandBuffer(commandBuffer)
        , m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(platformFactory, m_rendererScenes, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, rendererStatistics)
        , m_sceneStateExecutor(m_renderer, rendererSceneSender, m_rendererEventCollector)
        , m_rendererSceneUpdater(m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor)
        , m_sceneControlLogic(m_rendererSceneUpdater)
        , m_rendererCommandExecutor(m_renderer, m_rendererCommandBuffer, m_rendererSceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
        , m_sceneReferenceLogic(m_rendererScenes, m_sceneControlLogic, m_rendererSceneUpdater, rendererSceneSender)
        , m_cmdScreenshot                                  (m_rendererCommandBuffer)
        , m_cmdLogRendererInfo                             (m_rendererCommandBuffer)
        , m_cmdShowFrameProfiler                           (m_rendererCommandBuffer)
        , m_cmdPrintStatistics                             (m_rendererCommandBuffer)
        , m_cmdTriggerPickEvent                            (m_rendererCommandBuffer)
        , m_cmdSetClearColor                               (m_rendererCommandBuffer)
        , m_cmdSkippingOfUnmodifiedBuffers                 (m_rendererCommandBuffer)
        , m_cmdLinkSceneData                               (m_rendererCommandBuffer)
        , m_cmdUnlinkSceneData                             (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerListIviSurfaces         (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerSetLayerVisibility      (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerSetSurfaceVisibility    (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerSetSurfaceOpacity       (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerSetSurfaceDestRectangle (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerScreenshot        (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerAddSurfaceToLayer (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerRemoveSurfaceFromLayer (m_rendererCommandBuffer)
        , m_cmdSystemCompositorControllerDestroySurface (m_rendererCommandBuffer)
        , m_kpiMonitor(monitorFilename.empty() ? nullptr : new Monitor(monitorFilename))
        , m_cmdSetFrametimerValues(m_frameTimer)
    {
        m_rendererSceneUpdater.setSceneReferenceLogicHandler(m_sceneReferenceLogic);
        m_cmdShowSceneOnDisplayInternal.reset(new ShowSceneCommand(*this));
    }

    void WindowedRenderer::doOneLoop(ELoopMode loopMode, std::chrono::microseconds sleepTime)
    {
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

    void WindowedRenderer::update()
    {
        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::update() start update section of frame");
        m_frameTimer.startFrame();

        m_rendererCommandExecutor.executePendingCommands();
        updateSceneControlLogic();
        m_rendererSceneUpdater.updateScenes();
        m_renderer.updateSystemCompositorController();

        m_expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());

        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::update() end update section of frame");
    }

    void WindowedRenderer::render()
    {
        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::render() start render section of frame");

        for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
        {
            if (m_renderer.hasDisplayController(handle))
            {
                m_renderer.getDisplayController(handle).getRenderBackend().getDevice().resetDrawCallCount();
            }
        }

        m_renderer.doOneRenderLoop();
        m_rendererSceneUpdater.processScreenshotResults();

        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::render() end render section of frame");
    }

    void WindowedRenderer::collectEvents()
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };
        m_rendererEventCollector.appendAndConsumePendingEvents(m_rendererEvents, m_sceneControlEvents);

        if (std::max(m_rendererEvents.size(), m_sceneControlEvents.size()) > 1e6)
            LOG_ERROR(CONTEXT_RENDERER, "WindowedRenderer::collectEvents event queue has too many entries. "
                << "It seems application is not dispatching renderer events. Possible buffer overflow of the event queue! "
                << m_rendererEvents.size() << " renderer events, " << m_sceneControlEvents.size() << " scene events.");

        m_sceneReferenceLogic.extractAndSendSceneReferenceEvents(m_sceneControlEvents);
    }

    void WindowedRenderer::finishFrameStatistics(std::chrono::microseconds sleepTime)
    {
        const UInt32 drawCalls = static_cast<UInt32>(m_renderer.getProfilerStatistics().getCounterValues(FrameProfilerStatistics::ECounter::DrawCalls)[m_renderer.getProfilerStatistics().getCurrentFrameId()]);
        m_renderer.getStatistics().frameFinished(drawCalls);
        m_renderer.getProfilerStatistics().markFrameFinished(sleepTime);

        UInt32 drawCallCount(0u);
        UInt32 usedGPUMemory(0u);
        const IDevice* device = nullptr;
        for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
        {
            if (m_renderer.hasDisplayController(handle))
            {
                device = &m_renderer.getDisplayController(handle).getRenderBackend().getDevice();
                drawCallCount += device->getDrawCallCount();
                usedGPUMemory += device->getTotalGpuMemoryUsageInKB();
            }
        }

        m_renderer.getProfilerStatistics().setCounterValue(FrameProfilerStatistics::ECounter::DrawCalls, drawCallCount);
        m_renderer.getProfilerStatistics().setCounterValue(FrameProfilerStatistics::ECounter::UsedGPUMemory, usedGPUMemory / 1024);

        const UInt64 timeNowMs = PlatformTime::GetMillisecondsMonotonic();
        if (timeNowMs > m_lastUpdateTimeStampMilliSec + MonitorUpdateIntervalInMilliSec)
        {
            updateWindowTitles();

            const auto& stats = m_renderer.getStatistics();
            // TODO Violin KPI Monitor needs rework
            if (m_kpiMonitor)
            {
                const GpuMemorySample memorySample(m_rendererSceneUpdater);
                m_renderer.getMemoryStatistics().addMemorySample(memorySample);

                m_kpiMonitor->recordFrameInfo({ PlatformTime::GetMillisecondsAbsolute(), stats.getFps(), stats.getDrawCallsPerFrame(), usedGPUMemory });
            }
            LOG_TRACE_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        stats.writeStatsToStream(sos);
                        sos << "\n";
                        m_renderer.getProfilerStatistics().writeLongestFrameTimingsToStream(sos);
                    }));

            m_lastUpdateTimeStampMilliSec = timeNowMs;
        }
    }

    void WindowedRenderer::updateSceneControlLogic()
    {
        InternalSceneStateEvents internalSceneEvents;
        m_rendererEventCollector.dispatchInternalSceneStateEvents(internalSceneEvents);

        // pass only un/published events while inactive
        // if it ever becomes active then it will already know all the published scenes
        // if it does not become active then it will not interfere with legacy scene control
        if (!m_sceneControlLogicActive)
        {
            const auto it = std::remove_if(internalSceneEvents.begin(), internalSceneEvents.end(), [](const InternalSceneStateEvent& e)
            {
                return e.type != ERendererEventType_ScenePublished && e.type != ERendererEventType_SceneUnpublished;
            });
            internalSceneEvents.erase(it, internalSceneEvents.end());

            // Scene referencing cannot work properly with legacy control API.
            // If there is any active scene reference but not m_sceneControlLogicActive, it means legacy API was used
            if (m_sceneReferenceLogic.hasAnyReferencedScenes())
                LOG_ERROR(CONTEXT_RENDERER, "Scene referencing is used by some scene but deprecated API is used to control scene states - use RendererSceneControl instead of RendererSceneControl_legacy!");
        }

        for (const auto& evt : internalSceneEvents)
            m_sceneControlLogic.processInternalEvent(evt);

        RendererSceneControlLogic::Events outSceneEvents;
        m_sceneControlLogic.consumeEvents(outSceneEvents);
        for (const auto& evt : outSceneEvents)
        {
            switch (evt.type)
            {
            case RendererSceneControlLogic::Event::Type::ScenePublished:
                // TODO vaclav published is the only 'shared' event, it is already in general queue for legacy scene control, remove when legacy support gone
                //m_rendererEventCollector.addSceneEvent(ERendererEventType_ScenePublished, evt.sceneId, RendererSceneState::Unavailable);
                break;
            case RendererSceneControlLogic::Event::Type::SceneStateChanged:
                m_rendererEventCollector.addSceneEvent(ERendererEventType_SceneStateChanged, evt.sceneId, evt.state);
                break;
            }
        }
    }

    void WindowedRenderer::updateWindowTitles()
    {
        for (DisplayHandle handle(0u); handle < m_renderer.getDisplayControllerCount(); ++handle)
        {
            if (!m_renderer.hasDisplayController(handle))
            {
                continue;
            }

            IDisplayController& displayController = m_renderer.getDisplayController(handle);
            IWindow& window = displayController.getRenderBackend().getSurface().getWindow();

            if (window.hasTitle())
            {
                StringOutputStream stream;
                stream << "RAMSES Renderer ";

                stream << " | " << static_cast<Int>(m_renderer.getStatistics().getFps()) << " fps"
                       << " | " << EnumToString(displayController.getRenderBackend().getDevice().getDeviceTypeId())
                       << " | " << displayController.getDisplayWidth() << "x" << displayController.getDisplayHeight();

                window.setTitle(stream.release());
            }
        }
    }

    const Renderer& WindowedRenderer::getRenderer() const
    {
        return m_renderer;
    }

    Renderer& WindowedRenderer::getRenderer()
    {
        return m_renderer;
    }

    RendererCommandBuffer& WindowedRenderer::getRendererCommandBuffer()
    {
        return m_rendererCommandBuffer;
    }

    RendererEventCollector& WindowedRenderer::getEventCollector()
    {
        return m_rendererEventCollector;
    }

    const SceneStateExecutor& WindowedRenderer::getSceneStateExecutor() const
    {
        return m_sceneStateExecutor;
    }

    void WindowedRenderer::registerRamshCommands(Ramsh& ramsh)
    {
        ramsh.add(m_cmdPrintStatistics);
        ramsh.add(m_cmdTriggerPickEvent);
        ramsh.add(m_cmdSetClearColor);
        ramsh.add(m_cmdSkippingOfUnmodifiedBuffers);
        ramsh.add(m_cmdScreenshot);
        ramsh.add(m_cmdLogRendererInfo);
        ramsh.add(m_cmdShowFrameProfiler);
        ramsh.add(*m_cmdShowSceneOnDisplayInternal);
        ramsh.add(m_cmdLinkSceneData);
        ramsh.add(m_cmdUnlinkSceneData);
        ramsh.add(m_cmdSystemCompositorControllerListIviSurfaces);
        ramsh.add(m_cmdLinkSceneData);
        ramsh.add(m_cmdUnlinkSceneData);
        ramsh.add(m_cmdSystemCompositorControllerSetLayerVisibility);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceVisibility);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceOpacity);
        ramsh.add(m_cmdSystemCompositorControllerSetSurfaceDestRectangle);
        ramsh.add(m_cmdSystemCompositorControllerScreenshot);
        ramsh.add(m_cmdSystemCompositorControllerAddSurfaceToLayer);
        ramsh.add(m_cmdSystemCompositorControllerRemoveSurfaceFromLayer);
        ramsh.add(m_cmdSystemCompositorControllerDestroySurface);
        ramsh.add(m_cmdSetFrametimerValues);
    }

    void WindowedRenderer::dispatchRendererEvents(RendererEventVector& events)
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };
        events.swap(m_rendererEvents);
        m_rendererEvents.clear();
    }

    void WindowedRenderer::dispatchSceneControlEvents(RendererEventVector& events)
    {
        std::lock_guard<std::mutex> g{ m_eventsLock };
        events.swap(m_sceneControlEvents);
        m_sceneControlEvents.clear();
    }

    void WindowedRenderer::fireLoopTimingReportRendererEvent(std::chrono::microseconds maximumLoopTimeInPeriod, std::chrono::microseconds renderthreadAverageLooptime)
    {
        m_rendererEventCollector.addRenderStatsEvent(ERendererEventType_RenderThreadPeriodicLoopTimes, maximumLoopTimeInPeriod, renderthreadAverageLooptime);
    }

}
