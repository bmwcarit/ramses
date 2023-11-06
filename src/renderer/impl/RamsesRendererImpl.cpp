//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesRendererImpl.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RendererConfigImpl.h"
#include "impl/DisplayConfigImpl.h"
#include "impl/RendererSceneControlImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/BinaryShaderCacheProxy.h"
#include "impl/FrameworkFactoryRegistry.h"
#include "impl/RendererFactory.h"
#include "impl/ErrorReporting.h"
#include "impl/ValidationReportImpl.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Platform/PlatformFactory.h"
#include "internal/RendererLib/PlatformInterface/ISystemCompositorController.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/Ramsh/Ramsh.h"
#include <chrono>

namespace ramses::internal
{
    static const bool rendererRegisterSuccess = RendererFactory::RegisterRendererFactory();

    RamsesRendererImpl::RamsesRendererImpl(RamsesFrameworkImpl& framework, const ramses::RendererConfig& config)
        : m_framework(framework)
        , m_binaryShaderCache(config.impl().getBinaryShaderCache() ? new BinaryShaderCacheProxy(*(config.impl().getBinaryShaderCache())) : nullptr)
        , m_rendererFrameworkLogic(framework.getScenegraphComponent(), m_rendererCommandBuffer, framework.getFrameworkLock())
        , m_threadWatchdog(framework.getThreadWatchdogConfig(), ERamsesThreadIdentifier::Renderer)
        , m_displayDispatcher{ std::make_unique<DisplayDispatcher>(std::make_unique<PlatformFactory>(), config.impl().getInternalRendererConfig(), m_rendererFrameworkLogic, m_threadWatchdog) }
        , m_systemCompositorEnabled(config.impl().getInternalRendererConfig().getSystemCompositorControlEnabled())
        , m_loopMode(ELoopMode::UpdateAndRender)
        , m_rendererLoopThreadType(ERendererLoopThreadType_Undefined)
        , m_periodicLogSupplier(framework.getPeriodicLogger(), m_rendererCommandBuffer)
    {
        assert(!framework.isConnected());

        { //Add ramsh commands to ramsh, independent of whether it is enabled or not.
            m_ramshCommands.push_back(std::make_shared<Screenshot>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<CreateOffscreenBuffer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<LinkBuffer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<UnlinkBuffer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<AssignScene>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SetSceneState>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<LogRendererInfo>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<PrintStatistics>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<TriggerPickEvent>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SetClearColor>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SetSkippingOfUnmodifiedBuffers>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerListIviSurfaces>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerSetLayerVisibility>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerSetSurfaceVisibility>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerSetSurfaceOpacity>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerSetSurfaceDestRectangle>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerScreenshot>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerAddSurfaceToLayer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerRemoveSurfaceFromLayer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SystemCompositorControllerDestroySurface>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<SetFrameTimeLimits>(m_rendererCommandBuffer));
            for (const auto& cmd : m_ramshCommands)
                m_framework.getRamsh().add(cmd);
            LOG_DEBUG(CONTEXT_SMOKETEST, "Ramsh commands registered from RamsesRenderer");
        }

        LOG_TRACE(CONTEXT_PROFILING, "RamsesRenderer::RamsesRenderer finished initializing renderer");
    }

    RamsesRendererImpl::~RamsesRendererImpl() = default;

    bool RamsesRendererImpl::doOneLoop()
    {
        if (ERendererLoopThreadType_InRendererOwnThread == m_rendererLoopThreadType)
        {
            getErrorReporting().set("Can not call doOneLoop explicitly if renderer is (or was) running in its own thread!");
            return false;
        }

        m_rendererLoopThreadType = ERendererLoopThreadType_UsingDoOneLoop;
        m_displayDispatcher->dispatchCommands(m_rendererCommandBuffer);
        m_displayDispatcher->doOneLoop();
        return true;
    }

    bool RamsesRendererImpl::flush()
    {
        pushAndConsumeRendererCommands(m_pendingRendererCommands);
        return true;
    }

    displayId_t RamsesRendererImpl::createDisplay(const ramses::DisplayConfig& config)
    {
        ValidationReportImpl configReport;
        config.impl().validate(configReport);
        if (configReport.hasError())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createDisplay: failed to create display, using invalid display configuration - use validate method on object!");
            return displayId_t::Invalid();
        }

        const displayId_t displayId = m_nextDisplayId;
        m_nextDisplayId.getReference() = m_nextDisplayId.getValue() + 1;
        // display's framebuffer is also counted as display buffer together with offscreen buffers
        assert(m_displayFramebuffers.count(displayId) == 0);
        m_displayFramebuffers.insert({ displayId, m_nextDisplayBufferId });
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        RendererCommand::CreateDisplay cmd{ DisplayHandle(displayId.getValue()), config.impl().getInternalDisplayConfig(), m_binaryShaderCache.get() };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return displayId;
    }

    bool RamsesRendererImpl::destroyDisplay(displayId_t displayId)
    {
        RendererCommand::DestroyDisplay cmd{ DisplayHandle(displayId.getValue()) };
        m_pendingRendererCommands.push_back(std::move(cmd));
        m_displayFramebuffers.erase(displayId);

        return true;
    }

    displayBufferId_t RamsesRendererImpl::getDisplayFramebuffer(displayId_t displayId) const
    {
        const auto it = m_displayFramebuffers.find(displayId);
        if (it == m_displayFramebuffers.cend())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::getDisplayFramebuffer: there is no display with ID " << displayId);
            return displayBufferId_t::Invalid();
        }
        return it->second;
    }

    const DisplayDispatcher& RamsesRendererImpl::getDisplayDispatcher() const
    {
        return *m_displayDispatcher;
    }

    DisplayDispatcher& RamsesRendererImpl::getDisplayDispatcher()
    {
        return *m_displayDispatcher;
    }

    RendererSceneControl* RamsesRendererImpl::getSceneControlAPI()
    {
        if (!m_sceneControlAPI)
        {
            LOG_INFO(CONTEXT_CLIENT, "RamsesRenderer: instantiating RendererSceneControl");
            auto m_impl = std::make_unique<RendererSceneControlImpl>(*this);
            m_sceneControlAPI = UniquePtrWithDeleter<RendererSceneControl>{ new RendererSceneControl{ std::move(m_impl) }, [](RendererSceneControl* api) { delete api; } };
        }

        return m_sceneControlAPI.get();
    }

    void RamsesRendererImpl::logConfirmationEcho(displayId_t display, const std::string& text)
    {
        m_pendingRendererCommands.push_back(RendererCommand::ConfirmationEcho{ DisplayHandle{ display.getValue() }, text });
    }

    const RendererCommands& RamsesRendererImpl::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }

    displayBufferId_t RamsesRendererImpl::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, bool interruptible, EDepthBufferType depthBufferType)
    {
        if (width < 1u || height < 1u)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createOffscreenBuffer: failed to create offscreen buffer, resolution must be higher than 0x0!");
            return {};
        }

        const DisplayHandle displayHandle(display.getValue());
        const displayBufferId_t bufferId = m_nextDisplayBufferId;
        const OffscreenBufferHandle bufferHandle(bufferId.getValue());
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        RendererCommand::CreateOffscreenBuffer cmd{displayHandle, bufferHandle, width, height, sampleCount, interruptible, depthBufferType};
        m_pendingRendererCommands.push_back(std::move(cmd));

        return bufferId;
    }

    displayBufferId_t RamsesRendererImpl::createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers)
    {
        if(isThreaded())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createDmaOffscreenBuffer: failed to create offscreen buffer, renderer must be used only with doOneLoop (not running the renderer thread)!");
            return {};
        }

        if (width < 1u || height < 1u)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createDmaOffscreenBuffer: failed to create offscreen buffer, resolution must be higher than 0x0!");
            return {};
        }

        const DisplayHandle displayHandle(display.getValue());
        const displayBufferId_t bufferId = m_nextDisplayBufferId;
        const OffscreenBufferHandle bufferHandle(bufferId.getValue());
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        RendererCommand::CreateDmaOffscreenBuffer cmd{ displayHandle, bufferHandle, width, height, dmaBufferFourccFormat, dmaBufferUsageFlags, dmaBufferModifiers };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return bufferId;
    }

    bool RamsesRendererImpl::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const DisplayHandle displayHandle(display.getValue());
        const OffscreenBufferHandle bufferHandle(offscreenBuffer.getValue());
        m_pendingRendererCommands.push_back(RendererCommand::DestroyOffscreenBuffer{ displayHandle, bufferHandle });

        return true;
    }

    bool RamsesRendererImpl::setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, ClearFlags clearFlags)
    {
        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
        {
            getErrorReporting().set("RendererSceneControl::setDisplayBufferClearFlags failed: display does not exist.");
            return false;
        }

        OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = OffscreenBufferHandle::Invalid();

        const DisplayHandle displayHandle{ display.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::SetClearFlags{ displayHandle, bufferHandle, clearFlags });

        return true;
    }

    bool RamsesRendererImpl::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, const vec4f& color)
    {
        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
        {
            getErrorReporting().set("RendererSceneControl::setDisplayBufferClearColor failed: display does not exist.");
            return false;
        }

        OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = OffscreenBufferHandle::Invalid();

        const DisplayHandle displayHandle{ display.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::SetClearColor{ displayHandle, bufferHandle, color });

        return true;
    }

    bool RamsesRendererImpl::getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const
    {
        const auto it = std::find_if(m_offscreenDmaBufferInfos.cbegin(), m_offscreenDmaBufferInfos.cend(), [&](const auto& dmaBufInfo){ return dmaBufInfo.display == display && dmaBufInfo.displayBuffer == displayBufferId;});

        if (it == m_offscreenDmaBufferInfos.cend())
        {
            getErrorReporting().set(::fmt::format("RamsesRenderer::getDmaOffscreenBufferFDAndStride: no DMA buffer created for buffer {} on display {}", displayBufferId, display));
            return false;
        }

        fd = it->fd;
        stride = it->stride;

        return true;
    }

    streamBufferId_t RamsesRendererImpl::allocateStreamBuffer()
    {
        const streamBufferId_t bufferId = m_nextStreamBufferId;
        m_nextStreamBufferId.getReference() = m_nextStreamBufferId.getValue() + 1;
        return bufferId;
    }

    streamBufferId_t RamsesRendererImpl::createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source)
    {
        streamBufferId_t bufferId = allocateStreamBuffer();
        const DisplayHandle displayHandle{ display.getValue() };
        const StreamBufferHandle bufferHandle{ bufferId.getValue() };
        const WaylandIviSurfaceId sourceLL{ source.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::CreateStreamBuffer{ displayHandle, bufferHandle, sourceLL });

        return bufferId;
    }

    bool RamsesRendererImpl::destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer)
    {
        const DisplayHandle displayHandle{ display.getValue() };
        const StreamBufferHandle bufferHandle{ streamBuffer.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::DestroyStreamBuffer{ displayHandle, bufferHandle });

        return true;
    }

    externalBufferId_t RamsesRendererImpl::createExternalBuffer(displayId_t display)
    {
        if (isThreaded())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createExternalBuffer: can not create external buffers unless renderer is using doOneLoop");
            return {};
        }

        externalBufferId_t bufferId{ m_nextExternalBufferId };
        m_nextExternalBufferId.getReference() = m_nextExternalBufferId.getValue() + 1;

        const DisplayHandle displayHandle{ display.getValue() };
        const ExternalBufferHandle bufferHandle{ bufferId.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::CreateExternalBuffer{ displayHandle, bufferHandle });

        return bufferId;
    }

    bool RamsesRendererImpl::destroyExternalBuffer(displayId_t display, externalBufferId_t externalTexture)
    {
        const DisplayHandle displayHandle{ display.getValue() };
        const ExternalBufferHandle bufferHandle{ externalTexture.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::DestroyExternalBuffer{ displayHandle, bufferHandle });

        return true;
    }

    bool RamsesRendererImpl::readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        if (width == 0u || height == 0u)
        {
            getErrorReporting().set("RamsesRenderer::readPixels failed: width and height must be greater than Zero");
            return false;
        }
        const auto it = m_displayFramebuffers.find(displayId);
        if (it == m_displayFramebuffers.cend())
        {
            getErrorReporting().set("RamsesRenderer::readPixels failed: display does not exist.");
            return false;
        }

        OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to read from is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = OffscreenBufferHandle::Invalid();

        RendererCommand::ReadPixels cmd{ DisplayHandle{ displayId.getValue() }, bufferHandle, x, y, width, height, false, false, {} };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RamsesRendererImpl::systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::setSurfaceVisibility failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        const WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(RendererCommand::SCSetIviSurfaceVisibility{ waylandIviSurfaceId, visibility });
        return true;
    }

    bool RamsesRendererImpl::systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::setSurfaceOpacity failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        const WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(RendererCommand::SCSetIviSurfaceOpacity{ waylandIviSurfaceId, opacity });
        return true;
    }

    bool RamsesRendererImpl::systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::setSurfaceRectangle failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        const WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(RendererCommand::SCSetIviSurfaceDestRectangle{ waylandIviSurfaceId, x, y, width, height });
        return true;
    }

    bool RamsesRendererImpl::systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::setLayerVisibility failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        const WaylandIviLayerId waylandIviLayerId(layerId);
        m_pendingRendererCommands.push_back(RendererCommand::SCSetIviLayerVisibility{ waylandIviLayerId, visibility });
        return true;
    }

    bool RamsesRendererImpl::systemCompositorTakeScreenshot(std::string_view fileName, int32_t screenIviId)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::takeSystemCompositorScreenshot failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        m_pendingRendererCommands.push_back(RendererCommand::SCScreenshot{ screenIviId, std::string{fileName} });
        return true;
    }

    bool RamsesRendererImpl::systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId)
    {
        if (!m_systemCompositorEnabled)
        {
            getErrorReporting().set("RamsesRenderer::addSurfaceToLayer failed: system compositor was not enabled when creating the renderer.");
            return false;
        }

        const WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        const WaylandIviLayerId waylandIviLayerId(layerId);
        m_pendingRendererCommands.push_back(RendererCommand::SCAddIviSurfaceToIviLayer{ waylandIviSurfaceId, waylandIviLayerId });
        return true;
    }

    bool RamsesRendererImpl::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        m_tempRendererEvents.clear();
        m_displayDispatcher->dispatchRendererEvents(m_tempRendererEvents);

        for(const auto& event : m_tempRendererEvents)
        {
            switch (event.eventType)
            {
            case ERendererEventType::DisplayCreated:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult::Ok);
                break;
            case ERendererEventType::DisplayCreateFailed:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult::Failed);
                break;
            case ERendererEventType::DisplayDestroyed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult::Ok);
                break;
            case ERendererEventType::DisplayDestroyFailed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult::Failed);
                break;
            case ERendererEventType::ReadPixelsFromFramebuffer:
            case ERendererEventType::ReadPixelsFromFramebufferFailed:
            {
                const auto& pixelData = event.pixelData;
                const displayId_t displayId{ event.displayHandle.asMemoryHandle() };
                const OffscreenBufferHandle obHandle = event.offscreenBuffer;
                const displayBufferId_t displayBuffer(obHandle.isValid() ? obHandle.asMemoryHandle() : getDisplayFramebuffer(displayId).getValue());
                const auto eventResult = (event.eventType == ERendererEventType::ReadPixelsFromFramebuffer ? ERendererEventResult::Ok : ERendererEventResult::Failed);
                assert((event.eventType == ERendererEventType::ReadPixelsFromFramebuffer) ^ pixelData.empty());
                rendererEventHandler.framebufferPixelsRead(pixelData.data(), static_cast<uint32_t>(pixelData.size()), displayId, displayBuffer, eventResult);
                break;
            }
            case ERendererEventType::OffscreenBufferCreated:
            {
                const displayId_t display{ event.displayHandle.asMemoryHandle() };
                const displayBufferId_t displayBuffer{ event.offscreenBuffer.asMemoryHandle() };
                assert(std::find_if(m_offscreenDmaBufferInfos.cbegin(), m_offscreenDmaBufferInfos.cend(), [&](const auto& dmaBufInfo){ return dmaBufInfo.display == display && dmaBufInfo.displayBuffer == displayBuffer;}) == m_offscreenDmaBufferInfos.cend());
                m_offscreenDmaBufferInfos.push_back({ display, displayBuffer, event.dmaBufferFD, event.dmaBufferStride });

                rendererEventHandler.offscreenBufferCreated(display, displayBuffer, ERendererEventResult::Ok);
                break;
            }
            case ERendererEventType::OffscreenBufferCreateFailed:
                rendererEventHandler.offscreenBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult::Failed);
                break;
            case ERendererEventType::OffscreenBufferDestroyed:
            {
                const displayId_t display{ event.displayHandle.asMemoryHandle() };
                const displayBufferId_t displayBuffer{ event.offscreenBuffer.asMemoryHandle() };
                rendererEventHandler.offscreenBufferDestroyed(display, displayBuffer, ERendererEventResult::Ok);

                const auto it = std::find_if(m_offscreenDmaBufferInfos.cbegin(), m_offscreenDmaBufferInfos.cend(), [&](const auto& dmaBufInfo){ return dmaBufInfo.display == display && dmaBufInfo.displayBuffer == displayBuffer;});
                if(it != m_offscreenDmaBufferInfos.cend())
                    m_offscreenDmaBufferInfos.erase(it);
                break;
            }
            case ERendererEventType::OffscreenBufferDestroyFailed:
                rendererEventHandler.offscreenBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult::Failed);
                break;
            case ERendererEventType::ExternalBufferCreated:
                rendererEventHandler.externalBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, event.textureGlId, ERendererEventResult::Ok);
                break;
            case ERendererEventType::ExternalBufferCreateFailed:
                rendererEventHandler.externalBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, 0u, ERendererEventResult::Failed);
                break;
            case ERendererEventType::ExternalBufferDestroyed:
                rendererEventHandler.externalBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, ERendererEventResult::Ok);
                break;
            case ERendererEventType::ExternalBufferDestroyFailed:
                rendererEventHandler.externalBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, ERendererEventResult::Failed);
                break;
            case ERendererEventType::WindowClosed:
                rendererEventHandler.windowClosed(displayId_t{ event.displayHandle.asMemoryHandle() });
                break;
            case ERendererEventType::WindowKeyEvent:
                rendererEventHandler.keyEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    event.keyEvent.type, event.keyEvent.modifier, event.keyEvent.keyCode);
                break;
            case ERendererEventType::WindowMouseEvent:
                rendererEventHandler.mouseEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    event.mouseEvent.type,
                    event.mouseEvent.pos.x, event.mouseEvent.pos.y);
                break;
            case ERendererEventType::WindowResizeEvent:
                rendererEventHandler.windowResized(displayId_t{ event.displayHandle.asMemoryHandle() }, event.resizeEvent.width, event.resizeEvent.height);
                break;
            case ERendererEventType::WindowMoveEvent:
                rendererEventHandler.windowMoved(displayId_t{ event.displayHandle.asMemoryHandle() }, event.moveEvent.posX, event.moveEvent.posY);
                break;
            case ERendererEventType::FrameTimingReport:
                rendererEventHandler.renderThreadLoopTimings(displayId_t{ event.displayHandle.asMemoryHandle() }, event.frameTimings.maximumLoopTimeWithinPeriod, event.frameTimings.averageLoopTimeWithinPeriod);
                break;
            case ERendererEventType::Invalid:
            case ERendererEventType::ScenePublished:
            case ERendererEventType::SceneStateChanged:
            case ERendererEventType::SceneUnpublished:
            case ERendererEventType::SceneSubscribed:
            case ERendererEventType::SceneSubscribeFailed:
            case ERendererEventType::SceneUnsubscribed:
            case ERendererEventType::SceneUnsubscribedIndirect:
            case ERendererEventType::SceneUnsubscribeFailed:
            case ERendererEventType::SceneMapped:
            case ERendererEventType::SceneMapFailed:
            case ERendererEventType::SceneUnmapped:
            case ERendererEventType::SceneUnmappedIndirect:
            case ERendererEventType::SceneUnmapFailed:
            case ERendererEventType::SceneShown:
            case ERendererEventType::SceneShowFailed:
            case ERendererEventType::SceneHidden:
            case ERendererEventType::SceneHiddenIndirect:
            case ERendererEventType::SceneHideFailed:
            case ERendererEventType::SceneFlushed:
            case ERendererEventType::SceneExpirationMonitoringEnabled:
            case ERendererEventType::SceneExpirationMonitoringDisabled:
            case ERendererEventType::SceneExpired:
            case ERendererEventType::SceneRecoveredFromExpiration:
            case ERendererEventType::SceneDataLinked:
            case ERendererEventType::SceneDataLinkFailed:
            case ERendererEventType::SceneDataBufferLinked:
            case ERendererEventType::SceneDataBufferLinkFailed:
            case ERendererEventType::SceneDataUnlinked:
            case ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange:
            case ERendererEventType::SceneDataUnlinkFailed:
            case ERendererEventType::SceneDataSlotProviderCreated:
            case ERendererEventType::SceneDataSlotProviderDestroyed:
            case ERendererEventType::SceneDataSlotConsumerCreated:
            case ERendererEventType::SceneDataSlotConsumerDestroyed:
            case ERendererEventType::StreamSurfaceAvailable:
            case ERendererEventType::StreamSurfaceUnavailable:
            case ERendererEventType::ObjectsPicked:
                getErrorReporting().set("RamsesRenderer::dispatchEvents failed - unknown renderer event type!");
                assert(false);
                return false;
            }
        }

        return true;
    }

    bool RamsesRendererImpl::logRendererInfo()
    {
        m_pendingRendererCommands.push_back(RendererCommand::LogInfo{ ERendererLogTopic::All, true, {}, false, false, ELoopMode::UpdateAndRender, {} });
        return true;
    }

    bool RamsesRendererImpl::startThread()
    {
        if (m_rendererLoopThreadType == ERendererLoopThreadType_UsingDoOneLoop)
        {
            getErrorReporting().set("RamsesRenderer::startThread Can not call startThread if doOneLoop is called before!");
            return false;
        }

        m_displayDispatcher->startDisplayThreadsUpdating();
        m_diplayThreadUpdating = true;

        // First time starting thread, create dispatching thread.
        // Dispatching thread must be created after dispatcher startDisplayThreadsUpdating above which enables display threaded mode
        // and any existing queued up commands will be processed in threaded mode.
        if (m_rendererLoopThreadType == ERendererLoopThreadType_Undefined)
            m_commandDispatchingThread = std::make_unique<CommandDispatchingThread>(*m_displayDispatcher, m_rendererCommandBuffer, m_threadWatchdog);
        m_rendererLoopThreadType = ERendererLoopThreadType_InRendererOwnThread;

        return true;
    }

    bool RamsesRendererImpl::stopThread()
    {
        if (ERendererLoopThreadType_InRendererOwnThread != m_rendererLoopThreadType)
        {
            getErrorReporting().set("RamsesRenderer::stopThread Can not call stopThread if startThread was not called before!");
            return false;
        }

        m_displayDispatcher->stopDisplayThreadsUpdating();
        m_diplayThreadUpdating = false;

        return true;
    }

    bool RamsesRendererImpl::isThreadRunning() const
    {
        return m_diplayThreadUpdating;
    }

    bool RamsesRendererImpl::isThreaded() const
    {
        return m_rendererLoopThreadType == ERendererLoopThreadType_InRendererOwnThread;
    }

    bool RamsesRendererImpl::setFramerateLimit(displayId_t displayId, float fpsLimit)
    {
        if (fpsLimit <= 0.0f)
        {
            getErrorReporting().set("RamsesRenderer::setFramerateLimit must specify a positive fpsLimit!");
            return false;
        }
        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
        {
            getErrorReporting().set("RamsesRenderer::setFramerateLimit cannot call setFramerateLimit if doOneLoop is called before because it can only control framerate when using rendering thread!");
            return false;
        }

        m_displayDispatcher->setMinFrameDuration(std::chrono::microseconds{ std::lround(1000000 / fpsLimit) }, DisplayHandle{ displayId.getValue() });

        return true;
    }

    float RamsesRendererImpl::getFramerateLimit(displayId_t displayId) const
    {
        const auto minFrameDuration = m_displayDispatcher->getMinFrameDuration(DisplayHandle{ displayId.getValue() });
        return 1000000.f / static_cast<float>(minFrameDuration.count());
    }

    bool RamsesRendererImpl::setLoopMode(ELoopMode loopMode)
    {
        m_loopMode = loopMode;
        m_displayDispatcher->setLoopMode(m_loopMode);
        return true;
    }

    ELoopMode RamsesRendererImpl::getLoopMode() const
    {
        return m_loopMode;
    }

    bool RamsesRendererImpl::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_pendingRendererCommands.push_back(RendererCommand::SetLimits_FrameBudgets{ limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender });
        return true;
    }

    bool RamsesRendererImpl::setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height)
    {
        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
        {
            getErrorReporting().set("RamsesRenderer::setExternallyOwnedWindowSize failed: display does not exist.");
            return false;
        }

        m_pendingRendererCommands.push_back(RendererCommand::SetExterallyOwnedWindowSize{ DisplayHandle{ display.getValue() }, width, height });
        return true;
    }

    bool RamsesRendererImpl::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        m_pendingRendererCommands.push_back(RendererCommand::SetLimits_FlushesForceApply{ forceApplyFlushLimit });
        m_pendingRendererCommands.push_back(RendererCommand::SetLimits_FlushesForceUnsubscribe{ forceUnsubscribeSceneLimit });
        return true;
    }

    bool RamsesRendererImpl::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        m_pendingRendererCommands.push_back(RendererCommand::SetSkippingOfUnmodifiedBuffers{ enable });
        return true;
    }

    void RamsesRendererImpl::pushAndConsumeRendererCommands(RendererCommands& cmds)
    {
        m_rendererCommandBuffer.addAndConsumeCommandsFrom(cmds);
    }

    const RamsesRendererImpl::DisplayFrameBufferMap& RamsesRendererImpl::getDisplayFrameBuffers() const
    {
        return m_displayFramebuffers;
    }

    ErrorReporting& RamsesRendererImpl::getErrorReporting() const
    {
        return m_framework.getErrorReporting();
    }
}
