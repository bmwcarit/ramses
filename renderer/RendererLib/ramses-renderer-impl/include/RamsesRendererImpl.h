//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESRENDERERIMPL_H
#define RAMSES_RAMSESRENDERERIMPL_H

#include "ramses-renderer-api/Types.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/RendererSceneControl_legacy.h"
#include "ramses-renderer-api/DcsmContentControl.h"
#include "StatusObjectImpl.h"
#include "RendererLoopThreadController.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/WindowedRenderer.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/RendererPeriodicLogSupplier.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererFramework/RendererFrameworkLogic.h"
#include "Watchdog/PlatformWatchdog.h"
#include <memory>

namespace ramses_internal
{
    class RendererConfig;
    class IPlatformFactory;
    class IBinaryShaderCache;
}

namespace ramses
{
    class RamsesFrameworkImpl;
    class RendererConfig;
    class DisplayConfig;
    class IRendererEventHandler;
    class WarpingMeshData;

    class RamsesRendererImpl : public StatusObjectImpl
    {
    public:
        RamsesRendererImpl(RamsesFrameworkImpl& framework, const RendererConfig& config, ramses_internal::IPlatformFactory* platformFactory = nullptr);
        virtual ~RamsesRendererImpl();

        status_t doOneLoop();
        status_t flush();

        displayId_t createDisplay(const DisplayConfig& config);
        status_t    destroyDisplay(displayId_t displayId);
        displayBufferId_t getDisplayFramebuffer(displayId_t displayId) const;

        const ramses_internal::WindowedRenderer& getRenderer() const;
        ramses_internal::WindowedRenderer& getRenderer();

        RendererSceneControl* getSceneControlAPI();
        RendererSceneControl_legacy* getSceneControlAPI_legacy();
        DcsmContentControl* createDcsmContentControl(const DcsmContentControlConfig& config);

        displayBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, bool interruptible);
        status_t destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer);

        status_t readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        status_t updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData);

        status_t systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility);
        status_t systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity);
        status_t systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);
        status_t systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility);
        status_t systemCompositorTakeScreenshot(const char* fileName, int32_t screenIviId);
        status_t systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId);

        status_t dispatchEvents(IRendererEventHandler& rendererEventHandler);

        void logConfirmationEcho(const ramses_internal::String& text);
        status_t logRendererInfo();

        status_t startThread();
        status_t stopThread();
        bool isThreadRunning() const;
        bool isThreaded() const;
        status_t setMaximumFramerate(float maximumFramerate);
        float getMaximumFramerate() const;
        status_t setLoopMode(ELoopMode loopMode);
        ELoopMode getLoopMode() const;
        status_t setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);

        status_t setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);
        status_t setSkippingOfUnmodifiedBuffers(bool enable);

        status_t handlePickEvent(sceneId_t sceneId, float bufferNormalizedCoordX, float bufferNormalizedCoordY);

        const ramses_internal::RendererCommands& getPendingCommands() const;
        void submitRendererCommands(const ramses_internal::RendererCommands& cmds);

        using DisplayFrameBufferMap = std::unordered_map<displayId_t, displayBufferId_t>;
        const DisplayFrameBufferMap& getDisplayFrameBuffers() const;

    private:
        RamsesFrameworkImpl&                                                        m_framework;
        const ramses_internal::RendererConfig                                       m_internalConfig;
        std::unique_ptr<ramses_internal::IBinaryShaderCache>                        m_binaryShaderCache;
        std::unique_ptr<ramses_internal::IRendererResourceCache>                    m_rendererResourceCache;

        ramses_internal::RendererStatistics                                         m_rendererStatistics;

        ramses_internal::RendererCommands                                           m_pendingRendererCommands;
        ramses_internal::RendererCommandBuffer                                      m_rendererCommandBuffer;
        ramses_internal::RendererFrameworkLogic                                     m_rendererFrameworkLogic;
        std::unique_ptr<ramses_internal::IPlatformFactory>                          m_platformFactory;
        ramses_internal::ResourceUploader                                           m_resourceUploader;
        std::unique_ptr<ramses_internal::WindowedRenderer>                          m_renderer;

        displayId_t                                                                 m_nextDisplayId{ 0u };
        displayBufferId_t                                                           m_nextDisplayBufferId{ 0u };
        DisplayFrameBufferMap                                                       m_displayFramebuffers;
        bool                                                                        m_systemCompositorEnabled;
        ramses_internal::ELoopMode                                                  m_loopMode;
        ramses_internal::PlatformWatchdog                                           m_rendererLoopThreadWatchdog;
        ramses_internal::RendererLoopThreadController                               m_rendererLoopThreadController;

        enum ERendererLoopThreadType
        {
            ERendererLoopThreadType_Undefined = 0,
            ERendererLoopThreadType_InRendererOwnThread,
            ERendererLoopThreadType_UsingDoOneLoop
        };
        ERendererLoopThreadType                                                       m_rendererLoopThreadType;
        ramses_internal::RendererPeriodicLogSupplier                                  m_periodicLogSupplier;  //must be destructed before the RendererCommandBuffer!

        // scene control APIs can only be destructed within their friend RamsesRendererImpl class,
        // use custom deleter to achieve that with unique ptr
        template <typename T> using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;
        UniquePtrWithDeleter<RendererSceneControl> m_sceneControlAPI;
        UniquePtrWithDeleter<RendererSceneControl_legacy> m_sceneControlAPI_legacy;
        UniquePtrWithDeleter<DcsmContentControl> m_dcsmContentControl;

        // keep allocated containers which are used to swap internal data
        ramses_internal::RendererEventVector m_tempRendererEvents;
    };
}

#endif
