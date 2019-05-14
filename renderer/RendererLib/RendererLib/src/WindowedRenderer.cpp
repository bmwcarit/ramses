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
#include "Monitoring/Monitor.h"
#include "RendererLib/RendererCachedScene.h"
#include "Ramsh/Ramsh.h"
#include "Utils/Image.h"
#include "Utils/RamsesLogger.h"



namespace ramses_internal
{
    WindowedRenderer::WindowedRenderer(
        RendererCommandBuffer& commandBuffer,
        ISceneGraphConsumerComponent& sceneGraphConsumerComponent,
        IPlatformFactory& platformFactory,
        RendererStatistics& rendererStatistics,
        const String& monitorFilename)
        : m_rendererCommandBuffer(commandBuffer)
        , m_rendererScenes(m_rendererEventCollector)
        , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector)
        , m_renderer(platformFactory, m_rendererScenes, m_rendererEventCollector, m_frameTimer, m_expirationMonitor, rendererStatistics)
        , m_sceneStateExecutor(m_renderer, sceneGraphConsumerComponent, m_rendererEventCollector)
        , m_rendererSceneUpdater(m_renderer, m_rendererScenes, m_sceneStateExecutor, m_rendererEventCollector, m_frameTimer, m_expirationMonitor)
        , m_rendererCommandExecutor(m_renderer, m_rendererCommandBuffer, m_rendererSceneUpdater, m_rendererEventCollector, m_frameTimer)
        , m_cmdScreenshot                                  (m_rendererCommandBuffer)
        , m_cmdLogRendererInfo                             (m_rendererCommandBuffer)
        , m_cmdShowFrameProfiler                           (m_rendererCommandBuffer)
        , m_cmdPrintStatistics                             (m_rendererCommandBuffer)
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
        m_cmdShowSceneOnDisplayInternal.reset(new ShowSceneCommand(*this));
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

    WindowedRenderer::~WindowedRenderer()
    {
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

    const SceneStateExecutor& WindowedRenderer::getSceneStateExecutor() const
    {
        return m_sceneStateExecutor;
    }

    void WindowedRenderer::registerRamshCommands(Ramsh& ramsh)
    {
        ramsh.add(m_cmdPrintStatistics);
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
        m_rendererEventCollector.dispatchEvents(events);
    }

    void WindowedRenderer::update()
    {
        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::update() start update section of frame");
        m_frameTimer.startFrame();

        m_rendererCommandExecutor.executePendingCommands();
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
        processScreenshotResults();

        LOG_TRACE(CONTEXT_PROFILING, "WindowedRenderer::render() end render section of frame");
    }

    void WindowedRenderer::processScreenshotResults()
    {
        ScreenshotInfoVector screenshots;
        m_renderer.dispatchProcessedScreenshots(screenshots);

        for(auto& screenshot : screenshots)
        {
            if (screenshot.filename.getLength() > 0u)
            {
                if (screenshot.success)
                {
                    // flip image vertically so that the layout read from frame buffer (bottom-up)
                    // is converted to layout normally used in image files (top-down)
                    const Image bitmap((screenshot.rectangle.width - screenshot.rectangle.x), (screenshot.rectangle.height - screenshot.rectangle.y), screenshot.pixelData.cbegin(), screenshot.pixelData.cend(), true);
                    bitmap.saveToFilePNG(screenshot.filename);
                    LOG_INFO(CONTEXT_RENDERER, "RamsesRenderer::processScreenshots: screenshot successfully saved to file: " << screenshot.filename);
                    if (screenshot.sendViaDLT)
                    {
                        if (GetRamsesLogger().transmitFile(screenshot.filename, false))
                        {
                            LOG_INFO(CONTEXT_RENDERER, "RamsesRenderer::processScreenshots: screenshot file successfully send via dlt: " << screenshot.filename);
                        }
                        else
                        {
                            LOG_WARN(CONTEXT_RENDERER, "RamsesRenderer::processScreenshots: screenshot file could not send via dlt: " << screenshot.filename);
                        }
                    }
                }
                else
                {
                    LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::processScreenshots: failed screenshot! Not saved to file: " << screenshot.filename);
                }
            }
            else
            {
                if (screenshot.success)
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_ReadPixelsFromFramebuffer, screenshot.display, std::move(screenshot.pixelData));
                }
                else
                {
                    m_rendererEventCollector.addEvent(ERendererEventType_ReadPixelsFromFramebufferFailed, screenshot.display);
                }
            }
        }
    }
}
