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
#include "RendererLib/RendererStatistics.h"
#include "RendererLib/FrameProfilerStatistics.h"
#include "RendererLib/RendererInterruptState.h"
#include "RendererLib/DisplaySetup.h"
#include "RendererLib/DisplayEventHandler.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include <map>
#include <unordered_map>

namespace ramses_internal
{
    class IDisplayController;
    class RendererCachedScene;
    class ISystemCompositorController;
    class DisplayConfig;
    class IPlatform;
    class RendererScenes;
    class RendererEventCollector;
    class FrameTimer;
    class SceneExpirationMonitor;

    class Renderer
    {
        friend class RendererLogger;

    public:
        Renderer(
            DisplayHandle display,
            IPlatform& platform,
            const RendererScenes& rendererScenes,
            RendererEventCollector& eventCollector,
            const FrameTimer& frameTimer,
            SceneExpirationMonitor& expirationMonitor,
            RendererStatistics& rendererStatistics);
        virtual ~Renderer();

        void                        registerOffscreenBuffer    (DeviceResourceHandle bufferDeviceHandle, UInt32 width, UInt32 height, bool isInterruptible);
        void                        unregisterOffscreenBuffer  (DeviceResourceHandle bufferDeviceHandle);

        void                        doOneRenderLoop();

        void                        assignSceneToDisplayBuffer  (SceneId sceneId, DeviceResourceHandle buffer, Int32 globalSceneOrder);
        void                        unassignScene               (SceneId sceneId);
        [[nodiscard]] DeviceResourceHandle        getBufferSceneIsAssignedTo  (SceneId sceneId) const;
        [[nodiscard]] bool                        isSceneAssignedToInterruptibleOffscreenBuffer(SceneId sceneId) const;
        [[nodiscard]] Int32                       getSceneGlobalOrder         (SceneId sceneId) const;
        void                        setSceneShown               (SceneId sceneId, bool show);

        virtual void                markBufferWithSceneForRerender(SceneId sceneId);

        [[nodiscard]] const IDisplayController&   getDisplayController() const;
        IDisplayController&         getDisplayController();
        [[nodiscard]] bool                        hasDisplayController() const;
        [[nodiscard]] const DisplaySetup&         getDisplaySetup() const;
        void                        createDisplayContext(const DisplayConfig& displayConfig);
        void                        destroyDisplayContext();

        DisplayEventHandler&        getDisplayEventHandler();

        virtual void                setClearFlags(DeviceResourceHandle bufferDeviceHandle, uint32_t clearFlags);
        virtual void                setClearColor(DeviceResourceHandle bufferDeviceHandle, const glm::vec4& clearColor);
        virtual bool                setExternallyOwnedWindowSize(uint32_t width, uint32_t height);
        void                        scheduleScreenshot(DeviceResourceHandle renderTargetHandle, ScreenshotInfo&& screenshot);
        std::vector<std::pair<DeviceResourceHandle, ScreenshotInfo>> dispatchProcessedScreenshots();

        [[nodiscard]] bool                        hasAnyBufferWithInterruptedRendering() const;
        void                        resetRenderInterruptState();

        [[nodiscard]] bool hasSystemCompositorController() const;
        void updateSystemCompositorController() const;
        void systemCompositorListIviSurfaces() const;
        void systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility) const;
        void systemCompositorSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity) const;
        void systemCompositorSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height) const;
        void systemCompositorScreenshot(const String& fileName, int32_t screenIviId) const;
        void systemCompositorSetIviLayerVisibility(WaylandIviLayerId layerId, bool visibility) const;
        [[nodiscard]] bool systemCompositorAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const;
        void systemCompositorRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const;
        void systemCompositorDestroyIviSurface(WaylandIviSurfaceId surfaceId) const;

        //TODO: remove/refactor those functions
        RendererStatistics&         getStatistics();
        FrameProfilerStatistics&    getProfilerStatistics();

        static const glm::vec4 DefaultClearColor;

        // TODO vaclav remove, for debugging only
        std::atomic_int m_traceId{ 0 };

    protected:
        virtual IDisplayController* createDisplayControllerFromConfig(const DisplayConfig& config);

    private:
        void handleDisplayEvents();
        bool renderToFramebuffer();
        void renderToOffscreenBuffers();
        void renderToInterruptibleOffscreenBuffers();
        void processScheduledScreenshots(DeviceResourceHandle renderTargetHandle);
        void onSceneWasRendered(const RendererCachedScene& scene);

        DisplayHandle                          m_display;
        IPlatform&                             m_platform;

        std::unique_ptr<IDisplayController>    m_displayController;
        bool                                   m_canRenderFrame = true;
        DeviceResourceHandle                   m_frameBufferDeviceHandle;
        DisplaySetup                           m_displayBuffersSetup;
        std::unordered_map<DeviceResourceHandle, ScreenshotInfo> m_screenshots;

        const RendererScenes&                  m_rendererScenes;
        DisplayEventHandler                    m_displayEventHandler;

        RendererStatistics&                    m_statistics;
        FrameProfilerStatistics                m_profilerStatistics;

        RendererInterruptState                 m_rendererInterruptState;
        const FrameTimer&                      m_frameTimer;
        SceneExpirationMonitor&                m_expirationMonitor;

        // temporary containers kept to avoid re-allocations
        std::vector<SceneId> m_tempScenesToRender;
    };
}

#endif
