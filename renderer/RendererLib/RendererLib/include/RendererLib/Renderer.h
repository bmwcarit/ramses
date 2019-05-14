//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERER_H
#define RAMSES_RENDERER_H

#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"
#include "RendererAPI/EDeviceTypeId.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererLib/FrameProfilerStatistics.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/RendererInterruptState.h"
#include "RendererLib/DisplaySetup.h"
#include "FrameProfileRenderer.h"
#include "MemoryStatistics.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include <map>

namespace ramses_internal
{
    class IDisplayController;
    class RendererCachedScene;
    class ISystemCompositorController;
    class DisplayConfig;
    class IPlatformFactory;
    class IWindowEventsPollingManager;
    class RendererScenes;
    class DisplayEventHandler;
    class DisplayEventHandlerManager;
    class RendererEventCollector;
    class FrameTimer;
    class SceneExpirationMonitor;
    class WarpingMeshData;

    class Renderer
    {
        friend class RendererLogger;

    public:
        Renderer(IPlatformFactory& platformFactory, const RendererScenes& rendererScenes, RendererEventCollector& eventCollector, const FrameTimer& frameTimer, SceneExpirationMonitor& expirationMonitor, RendererStatistics& rendererStatistics);
        virtual ~Renderer();

        void                        registerOffscreenBuffer    (DisplayHandle display, DeviceResourceHandle bufferDeviceHandle, UInt32 width, UInt32 height, Bool isInterruptible);
        void                        unregisterOffscreenBuffer  (DisplayHandle display, DeviceResourceHandle bufferDeviceHandle);

        void                        doOneRenderLoop();

        void                        mapSceneToDisplayBuffer     (SceneId sceneId, DisplayHandle displayHandle, DeviceResourceHandle buffer, Int32 globalSceneOrder);
        void                        unmapScene                  (SceneId sceneId);
        DisplayHandle               getDisplaySceneIsMappedTo   (SceneId sceneId) const;
        DeviceResourceHandle        getBufferSceneIsMappedTo    (SceneId sceneId, DisplayHandle* displayHandleOut = nullptr) const;
        Bool                        isSceneMappedToInterruptibleOffscreenBuffer(SceneId sceneId) const;
        Int32                       getSceneGlobalOrder         (SceneId sceneId) const;
        void                        setSceneShown               (SceneId sceneId, Bool show);

        void                        markBufferWithMappedSceneAsModified(SceneId sceneId);
        void                        setSkippingOfUnmodifiedBuffers(Bool enable);

        virtual void                createDisplayContext(const DisplayConfig& displayConfig, DisplayHandle display);
        virtual void                destroyDisplayContext(DisplayHandle display);
        const IDisplayController&   getDisplayController(DisplayHandle display) const;
        IDisplayController&         getDisplayController(DisplayHandle display);
        UInt32                      getDisplayControllerCount() const;
        Bool                        hasDisplayController(DisplayHandle display) const;

        DisplayEventHandler&        getDisplayEventHandler(DisplayHandle display);
        void                        setWarpingMeshData(DisplayHandle display, const WarpingMeshData& meshData);

        void                        setClearColor(DisplayHandle displayHandle, const Vector4& clearColor);
        void                        scheduleScreenshot(const ScreenshotInfo& screenshot);
        void                        dispatchProcessedScreenshots(ScreenshotInfoVector& screenshots);

        Bool                        hasAnyBufferWithInterruptedRendering() const;
        void                        resetRenderInterruptState();

        FrameProfileRenderer&       getFrameProfileRenderer(DisplayHandle display);

        Bool hasSystemCompositorController() const;
        void updateSystemCompositorController() const;
        void systemCompositorListIviSurfaces() const;
        void systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility) const;
        void systemCompositorSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity) const;
        void systemCompositorSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height) const;
        void systemCompositorScreenshot(const String& fileName, int32_t screenIviId) const;
        void systemCompositorSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility) const;
        Bool systemCompositorAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const;
        void systemCompositorRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const;
        void systemCompositorDestroyIviSurface(WaylandIviSurfaceId surfaceId) const;

        //TODO: remove/refactor those functions
        RendererStatistics&         getStatistics();
        FrameProfilerStatistics&    getProfilerStatistics();
        MemoryStatistics&           getMemoryStatistics();

        static const Vector4 DefaultClearColor;

    protected:
        void addDisplayController(IDisplayController& display, DisplayHandle displayHandle);
        void removeDisplayController(DisplayHandle display);

    private:
        void handleDisplayEvents(DisplayHandle displayHandle);
        void renderToFramebuffer(DisplayHandle displayHandle, DisplayHandle& activeDisplay);
        void renderToOffscreenBuffers(DisplayHandle displayHandle, DisplayHandle& activeDisplay);
        void renderToInterruptibleOffscreenBuffers(DisplayHandle displayHandle, DisplayHandle& activeDisplay, Bool& interrupted);
        IDisplayController* createDisplayControllerFromConfig(const DisplayConfig& config, DisplayEventHandler& displayEventHandler);
        void processScheduledScreenshots(DisplayHandle display, IDisplayController& controller, DisplayHandle& activeDisplay);
        Bool hasAnyOffscreenBufferToRerender(DisplayHandle display, Bool interruptible) const;
        void onSceneWasRendered(const RendererCachedScene& scene);

        static void ActivateDisplayContext(DisplayHandle displayToActivate, DisplayHandle& activeDisplay, IDisplayController& dispController);
        static void ReorderDisplaysToStartWith(std::vector<DisplayHandle>& displays, DisplayHandle displayToStartWith);

        struct DisplayInfo
        {
            IDisplayController*  displayController;
            Bool                 couldRenderLastFrame;
            DeviceResourceHandle frameBufferDeviceHandle;
            DisplaySetup         buffersSetup;
        };
        using Displays = std::map<DisplayHandle, DisplayInfo>;

        IPlatformFactory&                      m_platformFactory;

        ISystemCompositorController*           m_systemCompositorController;
        const IWindowEventsPollingManager*     m_windowEventsPollingManager;
        Displays                               m_displays;

        const RendererScenes&                  m_rendererScenes;
        DisplayEventHandlerManager             m_displayHandlerManager;

        RendererStatistics&                    m_statistics;
        FrameProfilerStatistics                m_profilerStatistics;
        MemoryStatistics                       m_memoryStatistics;

        Bool                                   m_skipUnmodifiedBuffers = true;
        RendererInterruptState                 m_rendererInterruptState;
        const FrameTimer&                      m_frameTimer;
        SceneExpirationMonitor&                        m_expirationMonitor;

        HashMap<DisplayHandle, ScreenshotInfoVector> m_scheduledScreenshots;
        ScreenshotInfoVector m_processedScreenshots;

        using FrameProfilerMap = HashMap<DisplayHandle, FrameProfileRenderer*>;
        FrameProfilerMap m_frameProfileRenderer;

        // temporary containers kept to avoid re-allocations
        std::vector<DisplayHandle> m_tempDisplaysToRender; // used in RendererLogger - adapt if changing behavior
        std::vector<DisplayHandle> m_tempDisplaysToSwapBuffers;
        std::vector<SceneId> m_tempScenesRendered;
    };
}

#endif
