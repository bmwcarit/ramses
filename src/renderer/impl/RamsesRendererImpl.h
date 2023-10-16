//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/Types.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "impl/DataTypesImpl.h"
#include "impl/CommandDispatchingThread.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/RendererLib/DisplayDispatcher.h"
#include "internal/RendererLib/RendererPeriodicLogSupplier.h"
#include "internal/RendererLib/Enums/ELoopMode.h"
#include "internal/RendererLib/RendererFrameworkLogic.h"
#include "internal/Watchdog/ThreadWatchdog.h"
#include "internal/RendererLib/RamshCommands/Screenshot.h"
#include "internal/RendererLib/RamshCommands/LogRendererInfo.h"
#include "internal/RendererLib/RamshCommands/PrintStatistics.h"
#include "internal/RendererLib/RamshCommands/TriggerPickEvent.h"
#include "internal/RendererLib/RamshCommands/SetClearColor.h"
#include "internal/RendererLib/RamshCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerListIviSurfaces.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetLayerVisibility.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceVisibility.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceOpacity.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceDestRectangle.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerScreenshot.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerAddSurfaceToLayer.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerRemoveSurfaceFromLayer.h"
#include "internal/RendererLib/RamshCommands/SystemCompositorControllerDestroySurface.h"
#include "internal/RendererLib/RamshCommands/SetFrameTimeLimits.h"
#include "internal/RendererLib/RamshCommands/SetSceneState.h"
#include "internal/RendererLib/RamshCommands/LinkUnlink.h"
#include "internal/RendererLib/RamshCommands/CreateOffscreenBuffer.h"
#include "internal/RendererLib/RamshCommands/AssignScene.h"
#include <memory>
#include <string_view>
#include <string>

namespace ramses
{
    class RendererConfig;
    class DisplayConfig;
    class IRendererEventHandler;
}

namespace ramses::internal
{
    class IBinaryShaderCache;
    class RamsesFrameworkImpl;
    class ErrorReporting;

    class RamsesRendererImpl
    {
    public:
        RamsesRendererImpl(RamsesFrameworkImpl& framework, const ramses::RendererConfig& config);
        ~RamsesRendererImpl();

        bool doOneLoop();
        bool flush();

        displayId_t createDisplay(const ramses::DisplayConfig& config);
        bool destroyDisplay(displayId_t displayId);
        displayBufferId_t getDisplayFramebuffer(displayId_t displayId) const;
        const DisplayDispatcher& getDisplayDispatcher() const;
        DisplayDispatcher& getDisplayDispatcher();

        RendererSceneControl* getSceneControlAPI();

        displayBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, bool interruptible, EDepthBufferType depthBufferType);
        displayBufferId_t createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers);
        bool destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer);
        bool setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, ClearFlags clearFlags);
        bool setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, const vec4f& color);
        bool getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;

        streamBufferId_t allocateStreamBuffer();
        streamBufferId_t createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source);
        bool destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer);

        externalBufferId_t createExternalBuffer(displayId_t display);
        bool destroyExternalBuffer(displayId_t display, externalBufferId_t externalTexture);
        bool readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        bool systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility);
        bool systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity);
        bool systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);
        bool systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility);
        bool systemCompositorTakeScreenshot(std::string_view fileName, int32_t screenIviId);
        bool systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId);

        bool dispatchEvents(IRendererEventHandler& rendererEventHandler);

        void logConfirmationEcho(displayId_t display, const std::string& text);
        bool logRendererInfo();

        bool startThread();
        bool stopThread();
        bool isThreadRunning() const;
        bool isThreaded() const;
        bool setFramerateLimit(displayId_t displayId, float fpsLimit);
        float getFramerateLimit(displayId_t displayId) const;
        bool setLoopMode(ELoopMode loopMode);
        ELoopMode getLoopMode() const;
        bool setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        bool setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height);

        bool setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);
        bool setSkippingOfUnmodifiedBuffers(bool enable);

        const RendererCommands& getPendingCommands() const;
        void pushAndConsumeRendererCommands(RendererCommands& cmds);

        using DisplayFrameBufferMap = std::unordered_map<displayId_t, displayBufferId_t>;
        const DisplayFrameBufferMap& getDisplayFrameBuffers() const;

        ErrorReporting& getErrorReporting() const;

    private:
        RamsesFrameworkImpl&                                  m_framework;
        std::unique_ptr<ramses::internal::IBinaryShaderCache> m_binaryShaderCache;

        RendererCommands                   m_pendingRendererCommands;
        RendererCommandBuffer              m_rendererCommandBuffer;
        RendererFrameworkLogic             m_rendererFrameworkLogic;
        ThreadWatchdog                     m_threadWatchdog;
        std::unique_ptr<DisplayDispatcher> m_displayDispatcher;

        displayId_t           m_nextDisplayId{0u};
        displayBufferId_t     m_nextDisplayBufferId{0u};
        streamBufferId_t      m_nextStreamBufferId{0u};
        externalBufferId_t    m_nextExternalBufferId{0u};
        DisplayFrameBufferMap m_displayFramebuffers;
        bool                  m_systemCompositorEnabled;

        struct OffscreenDmaBufferInfo
        {
            displayId_t display;
            displayBufferId_t displayBuffer;
            int fd;
            uint32_t stride;
        };
        std::vector<OffscreenDmaBufferInfo> m_offscreenDmaBufferInfos;

        ELoopMode m_loopMode;
        std::unique_ptr<CommandDispatchingThread> m_commandDispatchingThread;
        bool m_diplayThreadUpdating = false;

        enum ERendererLoopThreadType
        {
            ERendererLoopThreadType_Undefined = 0,
            ERendererLoopThreadType_InRendererOwnThread,
            ERendererLoopThreadType_UsingDoOneLoop
        };
        ERendererLoopThreadType                                      m_rendererLoopThreadType;
        RendererPeriodicLogSupplier                                  m_periodicLogSupplier;  //must be destructed before the RendererCommandBuffer!

        // scene control APIs can only be destructed within their friend RamsesRendererImpl class,
        // use custom deleter to achieve that with unique ptr
        template <typename T> using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;
        UniquePtrWithDeleter<RendererSceneControl> m_sceneControlAPI;

        // keep allocated containers which are used to swap internal data
        RendererEventVector m_tempRendererEvents;

        std::vector < std::shared_ptr<RamshCommand> > m_ramshCommands;
    };
}
