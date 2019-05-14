//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOWEDRENDERER_H
#define RAMSES_WINDOWEDRENDERER_H

#include "RendererLib/DisplayController.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererCommandExecutor.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererCommands/Screenshot.h"
#include "RendererCommands/LogRendererInfo.h"
#include "RendererCommands/PrintStatistics.h"
#include "RendererCommands/SetClearColor.h"
#include "RendererCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "RendererCommands/ShowFrameProfiler.h"
#include "RendererCommands/ShowSceneCommand.h"
#include "RendererCommands/LinkSceneData.h"
#include "RendererCommands/UnlinkSceneData.h"
#include "RendererCommands/SystemCompositorControllerListIviSurfaces.h"
#include "RendererCommands/SystemCompositorControllerSetLayerVisibility.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceVisibility.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceOpacity.h"
#include "RendererCommands/SystemCompositorControllerSetSurfaceDestRectangle.h"
#include "RendererCommands/SystemCompositorControllerScreenshot.h"
#include "RendererCommands/SystemCompositorControllerAddSurfaceToLayer.h"
#include "RendererCommands/SystemCompositorControllerRemoveSurfaceFromLayer.h"
#include "RendererCommands/SystemCompositorControllerDestroySurface.h"
#include "RendererCommands/SetFrameTimeLimits.h"
#include "RendererEventCollector.h"
#include "DisplayEventHandlerManager.h"

namespace ramses_internal
{
    class RendererCommandBuffer;
    class Monitor;
    class Ramsh;
    class ISceneGraphConsumerComponent;
    class RendererStatistics;

    class WindowedRenderer
    {
    public:
        WindowedRenderer(
            RendererCommandBuffer& commandBuffer,
            ISceneGraphConsumerComponent& sceneGraphConsumerComponent,
            IPlatformFactory& platformFactory,
            RendererStatistics& m_rendererStatistics,
            const String& monitorFilename = String());
        ~WindowedRenderer();

        void update();
        void render();
        void finishFrameStatistics(std::chrono::microseconds sleepTime);

        const Renderer& getRenderer() const;
        Renderer& getRenderer();

        RendererCommandBuffer& getRendererCommandBuffer();
        const SceneStateExecutor& getSceneStateExecutor() const;

        void registerRamshCommands(Ramsh& ramsh);
        void dispatchRendererEvents(RendererEventVector& events);

    private:
        void updateWindowTitles();
        void processScreenshotResults();

        RendererCommandBuffer&                      m_rendererCommandBuffer;
        FrameTimer                                  m_frameTimer;
        RendererEventCollector                      m_rendererEventCollector;
        RendererScenes                              m_rendererScenes;
        SceneExpirationMonitor                      m_expirationMonitor;
        Renderer                                    m_renderer;
        SceneStateExecutor                          m_sceneStateExecutor;
        RendererSceneUpdater                        m_rendererSceneUpdater;
        RendererCommandExecutor                     m_rendererCommandExecutor;

        Screenshot                                        m_cmdScreenshot;
        LogRendererInfo                                   m_cmdLogRendererInfo;
        ShowFrameProfiler                                 m_cmdShowFrameProfiler;
        PrintStatistics                                   m_cmdPrintStatistics;
        SetClearColor                                     m_cmdSetClearColor;
        SetSkippingOfUnmodifiedBuffers                    m_cmdSkippingOfUnmodifiedBuffers;
        LinkSceneData                                     m_cmdLinkSceneData;
        UnlinkSceneData                                   m_cmdUnlinkSceneData;
        ScopedPointer<ShowSceneCommand>                   m_cmdShowSceneOnDisplayInternal;
        SystemCompositorControllerListIviSurfaces         m_cmdSystemCompositorControllerListIviSurfaces;
        SystemCompositorControllerSetLayerVisibility      m_cmdSystemCompositorControllerSetLayerVisibility;
        SystemCompositorControllerSetSurfaceVisibility    m_cmdSystemCompositorControllerSetSurfaceVisibility;
        SystemCompositorControllerSetSurfaceOpacity       m_cmdSystemCompositorControllerSetSurfaceOpacity;
        SystemCompositorControllerSetSurfaceDestRectangle m_cmdSystemCompositorControllerSetSurfaceDestRectangle;
        SystemCompositorControllerScreenshot              m_cmdSystemCompositorControllerScreenshot;
        SystemCompositorControllerAddSurfaceToLayer       m_cmdSystemCompositorControllerAddSurfaceToLayer;
        SystemCompositorControllerRemoveSurfaceFromLayer  m_cmdSystemCompositorControllerRemoveSurfaceFromLayer;
        SystemCompositorControllerDestroySurface          m_cmdSystemCompositorControllerDestroySurface;

        UInt64                                            m_lastUpdateTimeStampMilliSec = 0;
        static const UInt64                               MonitorUpdateIntervalInMilliSec = 500u;

        std::unique_ptr<Monitor>                          m_kpiMonitor;
        SetFrameTimeLimits                                m_cmdSetFrametimerValues;
    };
}

#endif
