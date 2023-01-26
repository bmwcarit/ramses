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
#include "ramses-renderer-api/DcsmContentControl.h"
#include "StatusObjectImpl.h"
#include "CommandDispatchingThread.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/DisplayDispatcher.h"
#include "RendererLib/RendererPeriodicLogSupplier.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererFramework/RendererFrameworkLogic.h"
#include "Watchdog/ThreadWatchdog.h"
#include "RendererCommands/Screenshot.h"
#include "RendererCommands/LogRendererInfo.h"
#include "RendererCommands/PrintStatistics.h"
#include "RendererCommands/TriggerPickEvent.h"
#include "RendererCommands/SetClearColor.h"
#include "RendererCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "RendererCommands/ShowFrameProfiler.h"
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
#include <memory>

namespace ramses_internal
{
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
        RamsesRendererImpl(RamsesFrameworkImpl& framework, const RendererConfig& config);

        status_t doOneLoop();
        status_t flush();

        displayId_t createDisplay(const DisplayConfig& config);
        status_t    destroyDisplay(displayId_t displayId);
        displayBufferId_t getDisplayFramebuffer(displayId_t displayId) const;
        const ramses_internal::DisplayDispatcher& getDisplayDispatcher() const;
        ramses_internal::DisplayDispatcher& getDisplayDispatcher();

        RendererSceneControl* getSceneControlAPI();
        DcsmContentControl* createDcsmContentControl();

        displayBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, bool interruptible, EDepthBufferType depthBufferType);
        displayBufferId_t createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, ramses_internal::DmaBufferFourccFormat dmaBufferFourccFormat, ramses_internal::DmaBufferUsageFlags dmaBufferUsageFlags, ramses_internal::DmaBufferModifiers dmaBufferModifiers);
        status_t destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer);
        status_t setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, uint32_t clearFlags);
        status_t setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a);
        status_t getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;

        streamBufferId_t allocateStreamBuffer();
        streamBufferId_t createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source);
        status_t destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer);
        status_t setStreamBufferState(displayId_t display, streamBufferId_t streamBufferId, bool state);

        externalBufferId_t createExternalBuffer(displayId_t display);
        status_t destroyExternalBuffer(displayId_t display, externalBufferId_t externalTexture);
        bool getExternalBufferGlId(displayId_t display, externalBufferId_t externalTexture, uint32_t& textureGlId) const;

        status_t readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        status_t updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData);

        status_t systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility);
        status_t systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity);
        status_t systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);
        status_t systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility);
        status_t systemCompositorTakeScreenshot(const char* fileName, int32_t screenIviId);
        status_t systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId);

        status_t dispatchEvents(IRendererEventHandler& rendererEventHandler);

        void logConfirmationEcho(displayId_t display, const ramses_internal::String& text);
        status_t logRendererInfo();

        status_t startThread();
        status_t stopThread();
        bool isThreadRunning() const;
        bool isThreaded() const;
        status_t setMaximumFramerate(float maximumFramerate);
        status_t setMaximumFramerate(float maximumFramerate, displayId_t display);
        float getMaximumFramerate() const;
        status_t setLoopMode(ELoopMode loopMode);
        ELoopMode getLoopMode() const;
        status_t setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        status_t setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height);

        status_t setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);
        status_t setSkippingOfUnmodifiedBuffers(bool enable);

        const ramses_internal::RendererCommands& getPendingCommands() const;
        void pushAndConsumeRendererCommands(ramses_internal::RendererCommands& cmds);

        using DisplayFrameBufferMap = std::unordered_map<displayId_t, displayBufferId_t>;
        const DisplayFrameBufferMap& getDisplayFrameBuffers() const;

    private:
        RamsesFrameworkImpl&                                                        m_framework;
        std::unique_ptr<ramses_internal::IBinaryShaderCache>                        m_binaryShaderCache;
        std::unique_ptr<ramses_internal::IRendererResourceCache>                    m_rendererResourceCache;

        ramses_internal::RendererCommands                                           m_pendingRendererCommands;
        ramses_internal::RendererCommandBuffer                                      m_rendererCommandBuffer;
        ramses_internal::RendererFrameworkLogic                                     m_rendererFrameworkLogic;
        ramses_internal::ThreadWatchdog                                             m_threadWatchdog;
        std::unique_ptr<ramses_internal::DisplayDispatcher>                         m_displayDispatcher;

        displayId_t                                                                 m_nextDisplayId{ 0u };
        displayBufferId_t                                                           m_nextDisplayBufferId{ 0u };
        streamBufferId_t                                                            m_nextStreamBufferId{ 0u };
        externalBufferId_t                                                          m_nextExternalBufferId{ 0u };
        DisplayFrameBufferMap                                                       m_displayFramebuffers;
        bool                                                                        m_systemCompositorEnabled;

        struct OffscreenDmaBufferInfo
        {
            displayId_t display;
            displayBufferId_t displayBuffer;
            int fd;
            uint32_t stride;
        };
        std::vector<OffscreenDmaBufferInfo>                                         m_offscreenDmaBufferInfos;

        struct ExternalBufferInfo
        {
            displayId_t display;
            externalBufferId_t externalBuffer;
            uint32_t glId;
        };
        std::vector<ExternalBufferInfo>                                             m_externalBufferInfos;

        ramses_internal::ELoopMode                                                  m_loopMode;
        std::unique_ptr<ramses_internal::CommandDispatchingThread>                  m_commandDispatchingThread;
        float m_maxFramerate = 1000000.f / ramses_internal::DefaultMinFrameDuration.count();
        bool m_diplayThreadUpdating = false;

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
        UniquePtrWithDeleter<DcsmContentControl> m_dcsmContentControl;

        // keep allocated containers which are used to swap internal data
        ramses_internal::RendererEventVector m_tempRendererEvents;

        std::vector < std::shared_ptr<ramses_internal::RamshCommand> > m_ramshCommands;
    };
}

#endif
