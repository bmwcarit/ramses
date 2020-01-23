//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesRendererImpl.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "RendererConfigImpl.h"
#include "DisplayConfigImpl.h"
#include "WarpingMeshDataImpl.h"
#include "SceneAPI/SceneId.h"
#include "Utils/LogMacros.h"
#include "Platform_Base/PlatformFactory_Base.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "BinaryShaderCacheProxy.h"
#include "RendererResourceCacheProxy.h"
#include "RamsesRendererUtils.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "RendererFactory.h"
#include "FrameworkFactoryRegistry.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    static const bool rendererRegisterSuccess = RendererFactory::RegisterRendererFactory();

    RamsesRendererImpl::RamsesRendererImpl(RamsesFrameworkImpl& framework, const RendererConfig& config, ramses_internal::IPlatformFactory* platformFactory)
        : StatusObjectImpl()
        , m_internalConfig(config.impl.getInternalRendererConfig())
        , m_binaryShaderCache(config.impl.getBinaryShaderCache() ? new BinaryShaderCacheProxy(*(config.impl.getBinaryShaderCache())) : nullptr)
        , m_rendererResourceCache(config.impl.getRendererResourceCache() ? new RendererResourceCacheProxy(*(config.impl.getRendererResourceCache())) : nullptr)
        , m_pendingRendererCommands()
        , m_rendererFrameworkLogic(framework.getRamsesConnectionStatusUpdateNotifier(), framework.getResourceComponent(), framework.getScenegraphComponent(), m_rendererCommandBuffer, framework.getFrameworkLock())
        , m_platformFactory(platformFactory != nullptr ? platformFactory : ramses_internal::PlatformFactory_Base::CreatePlatformFactory(m_internalConfig))
        , m_resourceUploader(m_rendererStatistics, m_binaryShaderCache.get())
        , m_renderer(new ramses_internal::WindowedRenderer(m_rendererCommandBuffer, framework.getScenegraphComponent(), *m_platformFactory, m_rendererStatistics, m_internalConfig.getKPIFileName()))
        , m_systemCompositorEnabled(m_internalConfig.getSystemCompositorControlEnabled())
        , m_loopMode(ramses_internal::ELoopMode_UpdateAndRender)
        , m_rendererLoopThreadWatchdog(framework.getThreadWatchdogConfig().getWatchdogNotificationInterval(ERamsesThreadIdentifier_Renderer), ERamsesThreadIdentifier_Renderer, framework.getThreadWatchdogConfig().getCallBack())
        , m_rendererLoopThreadController(*m_renderer, m_rendererLoopThreadWatchdog, m_internalConfig.getRenderThreadLoopTimingReportingPeriod())
        , m_rendererLoopThreadType(ERendererLoopThreadType_Undefined)
        , m_periodicLogSupplier(framework.getPeriodicLogger(), m_rendererCommandBuffer)
    {
        if (framework.isConnected())
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::RamsesRenderer creating a RamsesRenderer with framework which is already connected - this may lead to further issues! Please first create RamsesRenderer, then call connect() on RamsesFramework");
        }

        { //Add ramsh commands to ramsh, independent of whether it is enabled or not.

            m_renderer->registerRamshCommands(framework.getRamsh());
            LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Ramsh commands registered from RamsesRenderer");
        }

        LOG_TRACE(ramses_internal::CONTEXT_PROFILING, "RamsesRenderer::RamsesRenderer finished initializing renderer");
    }

    RamsesRendererImpl::~RamsesRendererImpl()
    {
        if (m_rendererLoopThreadType == ERendererLoopThreadType_InRendererOwnThread)
        {
            m_renderer.release();
            m_rendererLoopThreadController.stopRendering();
            m_rendererLoopThreadController.destroyRenderer();
        }
    }

    status_t RamsesRendererImpl::doOneLoop()
    {
        if (ERendererLoopThreadType_InRendererOwnThread == m_rendererLoopThreadType)
        {
            return addErrorEntry("Can not call doOneLoop explicitly if renderer is (or was) running in its own thread!");
        }

        m_rendererLoopThreadType = ERendererLoopThreadType_UsingDoOneLoop;
        RamsesRendererUtils::DoOneLoop(*m_renderer, m_loopMode, std::chrono::microseconds{ 0u });
        return StatusOK;
    }

    status_t RamsesRendererImpl::flush()
    {
        m_renderer->getRendererCommandBuffer().addCommands(m_pendingRendererCommands);
        m_pendingRendererCommands.clear();

        return StatusOK;
    }

    displayId_t RamsesRendererImpl::createDisplay(const DisplayConfig& config)
    {
        if (config.validate() != StatusOK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createDisplay: failed to create display, using invalid display configuration - use validate method on object!");
            return displayId_t::Invalid();
        }

        const displayId_t displayId = m_nextDisplayId;
        m_nextDisplayId.getReference() = m_nextDisplayId.getValue() + 1;
        // display's framebuffer is also counted as display buffer together with offscreen buffers
        assert(m_displayFramebuffers.count(displayId) == 0);
        m_displayFramebuffers.insert({ displayId, m_nextDisplayBufferId });
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        m_pendingRendererCommands.createDisplay(config.impl.getInternalDisplayConfig(), m_rendererFrameworkLogic, m_resourceUploader, ramses_internal::DisplayHandle(displayId.getValue()));

        return displayId;
    }

    status_t RamsesRendererImpl::destroyDisplay(displayId_t displayId)
    {
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_pendingRendererCommands.destroyDisplay(displayHandle);
        m_displayFramebuffers.erase(displayId);

        return StatusOK;
    }

    displayBufferId_t RamsesRendererImpl::getDisplayFramebuffer(displayId_t displayId) const
    {
        const auto it = m_displayFramebuffers.find(displayId);
        if (it == m_displayFramebuffers.cend())
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::getDisplayFramebuffer: there is no display with ID " << displayId);
            return displayBufferId_t::Invalid();
        }
        return it->second;
    }

    status_t RamsesRendererImpl::linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        if (providerSceneId == consumerSceneId)
        {
            return addErrorEntry("RamsesRenderer::linkData failed: provider- and consumer scene must not be identical");
        }

        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId.getValue());
        const ramses_internal::SceneId internalProviderSceneId(providerSceneId.getValue());
        m_pendingRendererCommands.linkSceneData(internalProviderSceneId, ramses_internal::DataSlotId(providerDataSlotId), internalConsumerSceneId, ramses_internal::DataSlotId(consumerDataSlotId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::OffscreenBufferHandle providerBuffer(offscreenBufferId.getValue());
        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId.getValue());
        m_pendingRendererCommands.linkBufferToSceneData(providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId(consumerDataSlotId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId.getValue());
        m_pendingRendererCommands.unlinkSceneData(internalConsumerSceneId, ramses_internal::DataSlotId(consumerDataSlotId));

        return StatusOK;
    }

    const ramses_internal::WindowedRenderer& RamsesRendererImpl::getRenderer() const
    {
        return *m_renderer;
    }

    ramses_internal::WindowedRenderer& RamsesRendererImpl::getRenderer()
    {
        return *m_renderer;
    }

    status_t RamsesRendererImpl::subscribeScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.subscribeScene(ramses_internal::SceneId(sceneId.getValue()));

        return StatusOK;
    }

    status_t RamsesRendererImpl::unsubscribeScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.unsubscribeScene(ramses_internal::SceneId(sceneId.getValue()), false);

        return StatusOK;
    }

    void RamsesRendererImpl::logConfirmationEcho(const ramses_internal::String& text)
    {
        m_pendingRendererCommands.confirmationEcho(text);
    }

    const ramses_internal::RendererCommands& RamsesRendererImpl::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }

    status_t RamsesRendererImpl::updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData)
    {
        if (newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() % 3 != 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: warping indices not divisible by 3 (not a triangle list)!");
        }

        assert(newWarpingMeshData.impl.getWarpingMeshData().getTextureCoordinates().size() == newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size());

        if (newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size() == 0 || newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() == 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: must provide more than zero indices and vertices!");
        }

        m_pendingRendererCommands.updateWarpingData(ramses_internal::DisplayHandle(displayId.getValue()), newWarpingMeshData.impl.getWarpingMeshData());

        return StatusOK;
    }

    status_t RamsesRendererImpl::mapScene(displayId_t displayId, sceneId_t sceneId)
    {
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        const ramses_internal::SceneId internalSceneId(sceneId.getValue());
        m_pendingRendererCommands.mapSceneToDisplay(internalSceneId, displayHandle);

        return StatusOK;
    }

    status_t RamsesRendererImpl::showScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.showScene(ramses_internal::SceneId(sceneId.getValue()));

        return StatusOK;
    }

    status_t RamsesRendererImpl::hideScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.hideScene(ramses_internal::SceneId(sceneId.getValue()));

        return StatusOK;
    }

    displayBufferId_t RamsesRendererImpl::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, bool interruptible)
    {
        if (width < 1u || width > 4096u ||
            height < 1u || height > 4096u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createOffscreenBuffer: failed to create offscreen buffer, resolution must be higher than 0x0 and lower than 4096x4096!");
            return {};
        }

        const ramses_internal::DisplayHandle displayHandle(display.getValue());
        const displayBufferId_t bufferId = m_nextDisplayBufferId;
        const ramses_internal::OffscreenBufferHandle bufferHandle(bufferId.getValue());
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        m_pendingRendererCommands.createOffscreenBuffer(bufferHandle, displayHandle, width, height, interruptible);

        return bufferId;
    }

    status_t RamsesRendererImpl::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const ramses_internal::DisplayHandle displayHandle(display.getValue());
        const ramses_internal::OffscreenBufferHandle bufferHandle(offscreenBuffer.getValue());
        m_pendingRendererCommands.destroyOffscreenBuffer(bufferHandle, displayHandle);

        return StatusOK;
    }

    status_t RamsesRendererImpl::assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        ramses_internal::OffscreenBufferHandle bufferHandle(displayBuffer.getValue());
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (std::any_of(m_displayFramebuffers.cbegin(), m_displayFramebuffers.cend(), [displayBuffer](const auto& d) { return d.second == displayBuffer; }))
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        const ramses_internal::SceneId internalSceneId(sceneId.getValue());
        m_pendingRendererCommands.assignSceneToDisplayBuffer(internalSceneId, bufferHandle, sceneRenderOrder);

        return StatusOK;
    }

    status_t RamsesRendererImpl::setBufferClearColor(displayId_t display, displayBufferId_t offscreenBuffer, float r, float g, float b, float a)
    {
        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
            return addErrorEntry("RamsesRenderer::setBufferClearColor failed: display does not exist.");

        ramses_internal::OffscreenBufferHandle bufferHandle(offscreenBuffer.getValue());
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (offscreenBuffer == it->second)
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        const ramses_internal::DisplayHandle displayHandle(display.getValue());
        m_pendingRendererCommands.setClearColor(displayHandle, bufferHandle, { r, g, b, a });

        return StatusOK;
    }

    status_t RamsesRendererImpl::unmapScene(sceneId_t sceneId)
    {
        const ramses_internal::SceneId internalSceneId(sceneId.getValue());
        m_pendingRendererCommands.unmapScene(internalSceneId);

        return StatusOK;
    }

    status_t RamsesRendererImpl::readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_pendingRendererCommands.readPixels(displayHandle, "", false, x, y, width, height);

        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        if (m_systemCompositorEnabled)
        {
            const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
            m_pendingRendererCommands.systemCompositorControllerSetIviSurfaceVisibility(waylandIviSurfaceId, visibility);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::setSurfaceVisibility failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity)
    {
        if (m_systemCompositorEnabled)
        {
            const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
            m_pendingRendererCommands.systemCompositorControllerSetIviSurfaceOpacity(waylandIviSurfaceId, opacity);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::setSurfaceOpacity failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        if (m_systemCompositorEnabled)
        {
            const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
            m_pendingRendererCommands.systemCompositorControllerSetIviSurfaceDestRectangle(waylandIviSurfaceId, x, y, width, height);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::setSurfaceRectangle failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility)
    {
        if (m_systemCompositorEnabled)
        {
            const ramses_internal::WaylandIviLayerId waylandIviLayerId(layerId);
            m_pendingRendererCommands.systemCompositorControllerSetIviLayerVisibility(waylandIviLayerId, visibility);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::setLayerVisibility failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorTakeScreenshot(const char* fileName, int32_t screenIviId)
    {
        if (m_systemCompositorEnabled)
        {
            m_pendingRendererCommands.systemCompositorControllerScreenshot(fileName, screenIviId);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::takeSystemCompositorScreenshot failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId)
    {
        if (m_systemCompositorEnabled)
        {
            const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
            const ramses_internal::WaylandIviLayerId waylandIviLayerId(layerId);
            m_pendingRendererCommands.systemCompositorControllerAddIviSurfaceToIviLayer(waylandIviSurfaceId, waylandIviLayerId);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::addSurfaceToLayer failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        ramses_internal::RendererEventVector events;
        m_renderer->dispatchRendererEvents(events);

        for(const auto& event : events)
        {
            switch (event.eventType)
            {
            case ramses_internal::ERendererEventType_ScenePublished:
                rendererEventHandler.scenePublished(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneUnpublished:
                rendererEventHandler.sceneUnpublished(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneFlushed:
                rendererEventHandler.sceneFlushed(sceneId_t(event.sceneId.getValue()), event.sceneVersionTag.getValue(),
                    RamsesRendererUtils::GetResourceStatus(event.resourceStatus));
                break;
            case ramses_internal::ERendererEventType_SceneSubscribed:
                rendererEventHandler.sceneSubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneSubscribeFailed:
                rendererEventHandler.sceneSubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribed:
                rendererEventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribedIndirect:
                rendererEventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribeFailed:
                rendererEventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneMapped:
                rendererEventHandler.sceneMapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneMapFailed:
                rendererEventHandler.sceneMapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapped:
                rendererEventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnmappedIndirect:
                rendererEventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapFailed:
                rendererEventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBuffer:
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBufferFailed:
            {
                displayBufferId_t bufferId{ event.offscreenBuffer.asMemoryHandle() };
                if (!event.offscreenBuffer.isValid())
                {
                    // if not assigned to offscreen buffer, it means it was assigned to display's framebuffer - find its HL id
                    const auto it = m_displayFramebuffers.find(displayId_t{ event.displayHandle.asMemoryHandle() });
                    if (it != m_displayFramebuffers.cend())
                        bufferId = it->second;
                }
                rendererEventHandler.sceneAssignedToDisplayBuffer(sceneId_t(event.sceneId.getValue()), bufferId,
                    (event.eventType == ramses_internal::ERendererEventType_SceneAssignedToDisplayBuffer ?  ERendererEventResult_OK : ERendererEventResult_FAIL));
                break;
            }
                break;
            case ramses_internal::ERendererEventType_SceneShown:
                rendererEventHandler.sceneShown(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneShowFailed:
                rendererEventHandler.sceneShown(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneHidden:
                rendererEventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneHiddenIndirect:
                rendererEventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneHideFailed:
                rendererEventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneExpired:
                rendererEventHandler.sceneExpired(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneRecoveredFromExpiration:
                rendererEventHandler.sceneRecoveredFromExpiration(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneDataLinked:
                rendererEventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), event.providerdataId.getValue(), sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataLinkFailed:
                rendererEventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), event.providerdataId.getValue(), sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinked:
                rendererEventHandler.offscreenBufferLinkedToSceneData(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinkFailed:
                rendererEventHandler.offscreenBufferLinkedToSceneData(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinked:
                rendererEventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange:
                rendererEventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkFailed:
                rendererEventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_DisplayCreated:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_DisplayCreateFailed:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_DisplayDestroyed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_DisplayDestroyFailed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_ReadPixelsFromFramebuffer:
            {
                const ramses_internal::UInt8Vector& pixelData = event.pixelData;
                assert(!pixelData.empty());
                rendererEventHandler.framebufferPixelsRead(&pixelData.front(), static_cast<uint32_t>(pixelData.size()), displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            }
            case ramses_internal::ERendererEventType_ReadPixelsFromFramebufferFailed:
                rendererEventHandler.framebufferPixelsRead(nullptr, 0u, displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_WarpingDataUpdated:
                rendererEventHandler.warpingMeshDataUpdated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_WarpingDataUpdateFailed:
                rendererEventHandler.warpingMeshDataUpdated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferCreated:
                rendererEventHandler.offscreenBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferCreateFailed:
                rendererEventHandler.offscreenBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferDestroyed:
                rendererEventHandler.offscreenBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferDestroyFailed:
                rendererEventHandler.offscreenBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderCreated:
                rendererEventHandler.dataProviderCreated(sceneId_t(event.providerSceneId.getValue()), event.providerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderDestroyed:
                rendererEventHandler.dataProviderDestroyed(sceneId_t(event.providerSceneId.getValue()), event.providerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerCreated:
                rendererEventHandler.dataConsumerCreated(sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerDestroyed:
                rendererEventHandler.dataConsumerDestroyed(sceneId_t(event.consumerSceneId.getValue()), event.consumerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_WindowClosed:
                rendererEventHandler.windowClosed(displayId_t{ event.displayHandle.asMemoryHandle() });
                break;
            case ramses_internal::ERendererEventType_WindowKeyEvent:
                rendererEventHandler.keyEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    RamsesRendererUtils::GetKeyEvent(event.keyEvent.type),
                    event.keyEvent.modifier, RamsesRendererUtils::GetKeyCode(event.keyEvent.keyCode));
                break;
            case ramses_internal::ERendererEventType_WindowMouseEvent:
                rendererEventHandler.mouseEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    RamsesRendererUtils::GetMouseEvent(event.mouseEvent.type),
                    event.mouseEvent.pos.x, event.mouseEvent.pos.y);
                break;
            case ramses_internal::ERendererEventType_WindowResizeEvent:
                rendererEventHandler.windowResized(displayId_t{ event.displayHandle.asMemoryHandle() }, event.resizeEvent.width, event.resizeEvent.height);
                break;
            case ramses_internal::ERendererEventType_WindowMoveEvent:
                rendererEventHandler.windowMoved(displayId_t{ event.displayHandle.asMemoryHandle() }, event.moveEvent.posX, event.moveEvent.posY);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceAvailable:
                rendererEventHandler.streamAvailabilityChanged(ramses::streamSource_t(event.streamSourceId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceUnavailable:
                rendererEventHandler.streamAvailabilityChanged(ramses::streamSource_t(event.streamSourceId.getValue()), false);
                break;
            case ramses_internal::ERendererEventType_ObjectsPicked:
                static_assert(sizeof(ramses::pickableObjectId_t) == sizeof(std::remove_pointer<decltype(event.pickedObjectIds.data())>::type), "");
                rendererEventHandler.objectsPicked(ramses::sceneId_t{ event.sceneId.getValue() }, reinterpret_cast<const ramses::pickableObjectId_t*>(event.pickedObjectIds.data()), static_cast<uint32_t>(event.pickedObjectIds.size()));
                break;
            case ramses_internal::ERendererEventType_RenderThreadPeriodicLoopTimes:
                rendererEventHandler.renderThreadLoopTimings(event.renderThreadLoopTimes.maximumLoopTimeWithinPeriod, event.renderThreadLoopTimes.averageLoopTimeWithinPeriod);
                break;
            default:
                assert(false);
                return addErrorEntry("RamsesRenderer::dispatchEvents failed - unknown renderer event type!");
            }
        }

        return StatusOK;
    }

    status_t RamsesRendererImpl::logRendererInfo()
    {
        m_pendingRendererCommands.logRendererInfo(ramses_internal::ERendererLogTopic_All, true, ramses_internal::NodeHandle::Invalid());
        return StatusOK;
    }

    status_t RamsesRendererImpl::startThread()
    {
        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
        {
            return addErrorEntry("RamsesRenderer::startThread Can not call startThread if doOneLoop is called before!");
        }

        m_rendererLoopThreadType = ERendererLoopThreadType_InRendererOwnThread;
        if (m_rendererLoopThreadController.startRendering())
        {
            return StatusOK;
        }

        return addErrorEntry("RamsesRenderer::startThread could not start rendering thread!");
    }

    status_t RamsesRendererImpl::stopThread()
    {
        if (ERendererLoopThreadType_InRendererOwnThread != m_rendererLoopThreadType)
        {
            return addErrorEntry("RamsesRenderer::stopThread Can not call stopThread if startThread was not called before!");
        }

        if (m_rendererLoopThreadController.stopRendering())
        {
            return StatusOK;
        }

        return addErrorEntry("RamsesRenderer::stopThread could not stop rendering thread!");
    }

    bool RamsesRendererImpl::isThreadRunning() const
    {
        return m_rendererLoopThreadController.isRendering();
    }

    bool RamsesRendererImpl::isThreaded() const
    {
        return m_rendererLoopThreadType == ERendererLoopThreadType_InRendererOwnThread;
    }

    status_t RamsesRendererImpl::setMaximumFramerate(float maximumFramerate)
    {
        if (maximumFramerate <= 0.0f)
        {
            return addErrorEntry("RamsesRenderer::setMaximumFramerate must specify a positive maximumFramerate!");
        }

        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
        {
            return addErrorEntry("RamsesRenderer::setMaximumFramerate Can not call setMaximumFramerate if doOneLoop is called before because it can only control framerate for rendering thread!");
        }

        m_rendererLoopThreadController.setMaximumFramerate(maximumFramerate);
        return StatusOK;
    }

    float RamsesRendererImpl::getMaximumFramerate() const
    {
        return m_rendererLoopThreadController.getMaximumFramerate();
    }

    status_t RamsesRendererImpl::setLoopMode(ELoopMode loopMode)
    {
        switch (loopMode)
        {
        case ELoopMode_UpdateAndRender:
            m_loopMode = ramses_internal::ELoopMode_UpdateAndRender;
            break;
        case ELoopMode_UpdateOnly:
            m_loopMode = ramses_internal::ELoopMode_UpdateOnly;
            break;
        default:
            assert(false);
            return addErrorEntry("RamsesRenderer::setLoopMode loop mode value not handled");
        }

        m_rendererLoopThreadController.setLoopMode(m_loopMode);

        return StatusOK;
    }

    ELoopMode RamsesRendererImpl::getLoopMode() const
    {
        return m_loopMode == ramses_internal::ELoopMode_UpdateAndRender? ramses::ELoopMode_UpdateAndRender : ramses::ELoopMode_UpdateOnly;
    }

    ramses::status_t RamsesRendererImpl::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
    {
        m_pendingRendererCommands.setFrameTimerLimits(limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
        return StatusOK;
    }

    status_t RamsesRendererImpl::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        m_pendingRendererCommands.setForceApplyPendingFlushesLimit(forceApplyFlushLimit);
        m_pendingRendererCommands.setForceUnsubscribeLimits(forceUnsubscribeSceneLimit);
        return StatusOK;
    }

    status_t RamsesRendererImpl::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        m_pendingRendererCommands.setSkippingOfUnmodifiedBuffers(enable);
        return StatusOK;
    }

    status_t RamsesRendererImpl::handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const ramses_internal::Vector2 coords(bufferNormalizedCoordX, bufferNormalizedCoordY);
        const ramses_internal::SceneId sceneId(scene.getValue());
        m_pendingRendererCommands.handlePickEvent(sceneId, coords);
        return StatusOK;
    }
}
