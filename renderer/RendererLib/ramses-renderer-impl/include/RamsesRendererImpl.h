//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESRENDERERIMPL_H
#define RAMSES_RAMSESRENDERERIMPL_H

#include "StatusObjectImpl.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/WindowedRenderer.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererFramework/RendererFrameworkLogic.h"
#include "RendererLib/RendererConfig.h"
#include "Watchdog/PlatformWatchdog.h"
#include "Utils/ScopedPointer.h"
#include "RendererLoopThreadController.h"
#include "RendererLib/RendererPeriodicLogSupplier.h"
#include "ramses-renderer-api/Types.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererLib/RendererStatistics.h"

namespace ramses_internal
{
    class RendererConfig;
    class IPlatformFactory;
    class IBinaryShaderCache;
    class DisplayEventHandler;
}

namespace ramses
{
    class RamsesFramework;
    class RendererConfig;
    class DisplayConfig;
    class SystemCompositorController;
    class IRendererEventHandler;
    class WarpingMeshData;

    class RamsesRendererImpl : public StatusObjectImpl
    {
    public:
        RamsesRendererImpl(RamsesFramework& framework, const RendererConfig& config, ramses_internal::IPlatformFactory* platformFactory = NULL);
        virtual ~RamsesRendererImpl();

        status_t doOneLoop();
        status_t flush();

        displayId_t createDisplay(const DisplayConfig& config);
        status_t    destroyDisplay(displayId_t displayId);

        status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        status_t linkOffscreenBufferToSceneData(offscreenBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        const ramses_internal::WindowedRenderer& getRenderer() const;
        ramses_internal::WindowedRenderer& getRenderer();

        status_t subscribeScene(sceneId_t sceneId);
        status_t unsubscribeScene(sceneId_t sceneId);

        status_t mapScene(displayId_t displayId, sceneId_t sceneId, int32_t sceneRenderOrder);
        status_t unmapScene(sceneId_t sceneId);

        status_t showScene(sceneId_t sceneId);
        status_t hideScene(sceneId_t sceneId);

        offscreenBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, bool interruptible);
        status_t            destroyOffscreenBuffer(displayId_t display, offscreenBufferId_t offscreenBuffer);

        status_t assignSceneToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBuffer);
        status_t assignSceneToFramebuffer(sceneId_t sceneId);

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

        const ramses_internal::RendererCommands& getPendingCommands() const;

        status_t startThread();
        status_t stopThread();
        bool isThreadRunning() const;
        bool isThreaded() const;
        status_t setMaximumFramerate(float maximumFramerate);
        float getMaximumFramerate() const;
        status_t setLoopMode(ELoopMode loopMode);
        ELoopMode getLoopMode() const;
        status_t setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender);

        status_t setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);
        status_t setSkippingOfUnmodifiedBuffers(bool enable);

    private:
        const ramses_internal::RendererConfig                                       m_internalConfig;
        ramses_internal::ScopedPointer<ramses_internal::IBinaryShaderCache>         m_binaryShaderCache;
        ramses_internal::ScopedPointer<ramses_internal::IRendererResourceCache>     m_rendererResourceCache;

        ramses_internal::RendererStatistics                                         m_rendererStatistics;

        ramses_internal::RendererCommands                                           m_pendingRendererCommands;
        ramses_internal::RendererCommandBuffer                                      m_rendererCommandBuffer;
        ramses_internal::RendererFrameworkLogic                                     m_rendererFrameworkLogic;
        ramses_internal::ScopedPointer<ramses_internal::IPlatformFactory>           m_platformFactory;
        ramses_internal::ResourceUploader                                           m_resourceUploader;
        ramses_internal::ScopedPointer<ramses_internal::WindowedRenderer>           m_renderer;

        displayId_t                                                                 m_nextDisplayId;
        offscreenBufferId_t                                                         m_nextOffscreenBufferId;
        bool                                                                        m_systemCompositorEnabled;
        ramses_internal::ELoopMode                                                  m_loopMode;
        mutable ramses_internal::PlatformLightweightLock                            m_lock;
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
    };
}

#endif
