//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/FrameProfilerStatistics.h"
#include "internal/RendererLib/RendererInterruptState.h"
#include "internal/RendererLib/DisplaySetup.h"
#include "internal/RendererLib/DisplayEventHandler.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

#include <map>
#include <unordered_map>
#include <string_view>

namespace ramses::internal
{
    class IDisplayController;
    class RendererCachedScene;
    class ISystemCompositorController;
    class DisplayConfigData;
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

        void                        registerOffscreenBuffer    (DeviceResourceHandle bufferDeviceHandle, uint32_t width, uint32_t height, uint32_t sampleCount, bool isInterruptible);
        void                        unregisterOffscreenBuffer  (DeviceResourceHandle bufferDeviceHandle);

        void                        doOneRenderLoop();

        void                        assignSceneToDisplayBuffer  (SceneId sceneId, DeviceResourceHandle buffer, int32_t globalSceneOrder);
        void                        unassignScene               (SceneId sceneId);
        [[nodiscard]] DeviceResourceHandle        getBufferSceneIsAssignedTo  (SceneId sceneId) const;
        [[nodiscard]] bool                        isSceneAssignedToInterruptibleOffscreenBuffer(SceneId sceneId) const;
        [[nodiscard]] int32_t                       getSceneGlobalOrder         (SceneId sceneId) const;
        void                        setSceneShown               (SceneId sceneId, bool show);

        virtual void                markBufferWithSceneForRerender(SceneId sceneId);

        [[nodiscard]] const IDisplayController&   getDisplayController() const;
        IDisplayController&         getDisplayController();
        [[nodiscard]] bool                        hasDisplayController() const;
        [[nodiscard]] const DisplaySetup&         getDisplaySetup() const;
        void                        createDisplayContext(const DisplayConfigData& displayConfig);
        void                        destroyDisplayContext();

        DisplayEventHandler&        getDisplayEventHandler();

        virtual void                setClearFlags(DeviceResourceHandle bufferDeviceHandle, ClearFlags clearFlags);
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
        void systemCompositorSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, float opacity) const;
        void systemCompositorSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, int32_t x, int32_t y, int32_t width, int32_t height) const;
        void systemCompositorScreenshot(std::string_view fileName, int32_t screenIviId) const;
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
        virtual IDisplayController* createDisplayControllerFromConfig(const DisplayConfigData& config);

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
