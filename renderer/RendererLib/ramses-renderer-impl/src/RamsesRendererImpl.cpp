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
#include "Common/Cpp11Macros.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    RamsesRendererImpl::RamsesRendererImpl(RamsesFramework& framework, const RendererConfig& config, ramses_internal::IPlatformFactory* platformFactory)
        : StatusObjectImpl()
        , m_internalConfig(config.impl.getInternalRendererConfig())
        , m_binaryShaderCache(config.impl.getBinaryShaderCache() ? new BinaryShaderCacheProxy(*(config.impl.getBinaryShaderCache())) : NULL)
        , m_rendererResourceCache(config.impl.getRendererResourceCache() ? new RendererResourceCacheProxy(*(config.impl.getRendererResourceCache())) : nullptr)
        , m_pendingRendererCommands()
        , m_rendererFrameworkLogic(framework.impl.getConnectionStatusUpdateNotifier(), framework.impl.getResourceComponent(), framework.impl.getScenegraphComponent(), m_rendererCommandBuffer, framework.impl.getFrameworkLock())
        , m_platformFactory(platformFactory != NULL ? platformFactory : ramses_internal::PlatformFactory_Base::CreatePlatformFactory(m_internalConfig))
        , m_resourceUploader(m_rendererStatistics, m_binaryShaderCache.get())
        , m_renderer(new ramses_internal::WindowedRenderer(m_rendererCommandBuffer, framework.impl.getScenegraphComponent(), *m_platformFactory, m_rendererStatistics, m_internalConfig.getKPIFileName()))
        , m_nextDisplayId(0u)
        , m_nextOffscreenBufferId(0u)
        , m_systemCompositorEnabled(m_internalConfig.getSystemCompositorControlEnabled())
        , m_loopMode(ramses_internal::ELoopMode_UpdateAndRender)
        , m_lock()
        , m_rendererLoopThreadWatchdog(framework.impl.getThreadWatchdogConfig().getWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Renderer), ramses::ERamsesThreadIdentifier_Renderer, framework.impl.getThreadWatchdogConfig().getCallBack())
        , m_rendererLoopThreadController(*m_renderer, m_rendererLoopThreadWatchdog)
        , m_rendererLoopThreadType(ERendererLoopThreadType_Undefined)
        , m_periodicLogSupplier(framework.impl.getPeriodicLogger(), m_rendererCommandBuffer)
    {
        if (framework.isConnected())
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::RamsesRenderer creating a RamsesRenderer with framework which is already connected - this may lead to further issues! Please first create RamsesRenderer, then call connect() on RamsesFramework");
        }

        { //Add ramsh commands to ramsh, independent of whether it is enabled or not.

            m_renderer->registerRamshCommands(framework.impl.getRamsh());
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
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (ERendererLoopThreadType_InRendererOwnThread == m_rendererLoopThreadType)
        {
            return addErrorEntry("Can not call doOneLoop explicitly if renderer is (or was) running in its own thread!");
        }

        m_rendererLoopThreadType = ERendererLoopThreadType_UsingDoOneLoop;
        RamsesRendererUtils::DoOneLoop(*m_renderer, m_loopMode, std::chrono::microseconds{ 0u });
        return StatusOK;
    }

    ramses::status_t RamsesRendererImpl::flush()
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        m_renderer->getRendererCommandBuffer().addCommands(m_pendingRendererCommands);
        m_pendingRendererCommands.clear();

        return StatusOK;
    }

    displayId_t RamsesRendererImpl::createDisplay(const DisplayConfig& config)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (config.validate() != StatusOK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createDisplay: failed to create display, using invalid display configuration - use validate method on object!");
            return InvalidDisplayId;
        }

        const displayId_t displayId = m_nextDisplayId;
        m_pendingRendererCommands.createDisplay(config.impl.getInternalDisplayConfig(), m_rendererFrameworkLogic, m_resourceUploader, ramses_internal::DisplayHandle(displayId));

        ++m_nextDisplayId;
        // sanity check
        if (m_nextDisplayId > 1e9)
        {
            m_nextDisplayId = 0u;
        }

        return displayId;
    }

    status_t RamsesRendererImpl::destroyDisplay(displayId_t displayId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::DisplayHandle displayHandle(displayId);
        m_pendingRendererCommands.destroyDisplay(displayHandle);

        return StatusOK;
    }

    status_t RamsesRendererImpl::linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (providerSceneId == consumerSceneId)
        {
            return addErrorEntry("RamsesRenderer::linkData failed: provider- and consumer scene must not be identical");
        }

        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId);
        const ramses_internal::SceneId internalProviderSceneId(providerSceneId);
        m_pendingRendererCommands.linkSceneData(internalProviderSceneId, ramses_internal::DataSlotId(providerDataSlotId), internalConsumerSceneId, ramses_internal::DataSlotId(consumerDataSlotId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::linkOffscreenBufferToSceneData(offscreenBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::OffscreenBufferHandle providerBuffer(offscreenBufferId);
        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId);
        m_pendingRendererCommands.linkBufferToSceneData(providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId(consumerDataSlotId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::SceneId internalConsumerSceneId(consumerSceneId);
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
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.subscribeScene(ramses_internal::SceneId(sceneId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::unsubscribeScene(sceneId_t sceneId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.unsubscribeScene(ramses_internal::SceneId(sceneId), false);

        return StatusOK;
    }

    void RamsesRendererImpl::logConfirmationEcho(const ramses_internal::String& text)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.confirmationEcho(text);
    }

    const ramses_internal::RendererCommands& RamsesRendererImpl::getCommands() const
    {
        return m_pendingRendererCommands;
    }

    status_t RamsesRendererImpl::updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() % 3 != 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: warping indices not divisible by 3 (not a triangle list)!");
        }

        assert(newWarpingMeshData.impl.getWarpingMeshData().getTextureCoordinates().size() == newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size());

        if (newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size() == 0 || newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() == 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: must provide more than zero indices and vertices!");
        }

        m_pendingRendererCommands.updateWarpingData(ramses_internal::DisplayHandle(displayId), newWarpingMeshData.impl.getWarpingMeshData());

        return StatusOK;
    }

    status_t RamsesRendererImpl::mapScene(displayId_t displayId, sceneId_t sceneId, int32_t sceneRenderOrder)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::DisplayHandle displayHandle(displayId);
        const ramses_internal::SceneId internalSceneId(sceneId);
        m_pendingRendererCommands.mapSceneToDisplay(internalSceneId, displayHandle, sceneRenderOrder);

        return StatusOK;
    }

    status_t RamsesRendererImpl::showScene(sceneId_t sceneId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.showScene(ramses_internal::SceneId(sceneId));

        return StatusOK;
    }

    status_t RamsesRendererImpl::hideScene(sceneId_t sceneId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.hideScene(ramses_internal::SceneId(sceneId));

        return StatusOK;
    }

    offscreenBufferId_t RamsesRendererImpl::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, bool interruptible)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (width < 1u || width > 4096u ||
            height < 1u || height > 4096u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createOffscreenBuffer: failed to create offscreen buffer, resolution must be higher than 0x0 and lower than 4096x4096!");
            return InvalidOffscreenBufferId;
        }

        const ramses_internal::DisplayHandle displayHandle(display);
        const offscreenBufferId_t bufferId = m_nextOffscreenBufferId;
        const ramses_internal::OffscreenBufferHandle bufferHandle(bufferId);
        m_pendingRendererCommands.createOffscreenBuffer(bufferHandle, displayHandle, width, height, interruptible);

        ++m_nextOffscreenBufferId;
        // sanity check
        if (m_nextOffscreenBufferId > 1e9)
        {
            m_nextOffscreenBufferId = 0u;
        }

        return bufferId;
    }

    status_t RamsesRendererImpl::destroyOffscreenBuffer(displayId_t display, offscreenBufferId_t offscreenBuffer)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::DisplayHandle displayHandle(display);
        const ramses_internal::OffscreenBufferHandle bufferHandle(offscreenBuffer);
        m_pendingRendererCommands.destroyOffscreenBuffer(bufferHandle, displayHandle);

        return StatusOK;
    }

    status_t RamsesRendererImpl::assignSceneToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBuffer)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::SceneId internalSceneId(sceneId);
        const ramses_internal::OffscreenBufferHandle internalBuffer(offscreenBuffer);
        m_pendingRendererCommands.assignSceneToOffscreenBuffer(internalSceneId, internalBuffer);

        return StatusOK;
    }

    status_t RamsesRendererImpl::assignSceneToFramebuffer(sceneId_t sceneId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::SceneId internalSceneId(sceneId);
        m_pendingRendererCommands.assignSceneToFramebuffer(internalSceneId);

        return StatusOK;
    }

    status_t RamsesRendererImpl::unmapScene(sceneId_t sceneId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        const ramses_internal::SceneId internalSceneId(sceneId);
        m_pendingRendererCommands.unmapScene(internalSceneId);

        return StatusOK;
    }

    status_t RamsesRendererImpl::readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        const ramses_internal::DisplayHandle displayHandle(displayId);
        m_pendingRendererCommands.readPixels(displayHandle, "", false, x, y, width, height);

        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

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
        ramses_internal::PlatformLightweightGuard guard(m_lock);

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
        ramses_internal::PlatformLightweightGuard guard(m_lock);

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

    status_t RamsesRendererImpl::systemCompositorTakeScreenshot(const char* fileName)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

        if (m_systemCompositorEnabled)
        {
            m_pendingRendererCommands.systemCompositorControllerScreenshot(fileName);
            return StatusOK;
        }
        else
        {
            return addErrorEntry("RamsesRenderer::takeSystemCompositorScreenshot failed: system compositor was not enabled when creating the renderer.");
        }
    }

    status_t RamsesRendererImpl::systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

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

        ramses_foreach(events, rendererEvent)
        {
            switch (rendererEvent->eventType)
            {
            case ramses_internal::ERendererEventType_ScenePublished:
                rendererEventHandler.scenePublished(rendererEvent->sceneId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneUnpublished:
                rendererEventHandler.sceneUnpublished(rendererEvent->sceneId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneFlushed:
                rendererEventHandler.sceneFlushed(rendererEvent->sceneId.getValue(), rendererEvent->sceneVersionTag.getValue(),
                    RamsesRendererUtils::GetResourceStatus(rendererEvent->resourceStatus));
                break;
            case ramses_internal::ERendererEventType_SceneSubscribed:
                rendererEventHandler.sceneSubscribed(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneSubscribeFailed:
                rendererEventHandler.sceneSubscribed(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribed:
                rendererEventHandler.sceneUnsubscribed(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribedIndirect:
                rendererEventHandler.sceneUnsubscribed(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribeFailed:
                rendererEventHandler.sceneUnsubscribed(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneMapped:
                rendererEventHandler.sceneMapped(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneMapFailed:
                rendererEventHandler.sceneMapped(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapped:
                rendererEventHandler.sceneUnmapped(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnmappedIndirect:
                rendererEventHandler.sceneUnmapped(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapFailed:
                rendererEventHandler.sceneUnmapped(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToOffscreenBuffer:
                rendererEventHandler.sceneAssignedToOffscreenBuffer(rendererEvent->sceneId.getValue(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToOffscreenBufferFailed:
                rendererEventHandler.sceneAssignedToOffscreenBuffer(rendererEvent->sceneId.getValue(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToFramebuffer:
                rendererEventHandler.sceneAssignedToFramebuffer(rendererEvent->sceneId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToFramebufferFailed:
                rendererEventHandler.sceneAssignedToFramebuffer(rendererEvent->sceneId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneShown:
                rendererEventHandler.sceneShown(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneShowFailed:
                rendererEventHandler.sceneShown(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneHidden:
                rendererEventHandler.sceneHidden(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneHiddenIndirect:
                rendererEventHandler.sceneHidden(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneHideFailed:
                rendererEventHandler.sceneHidden(rendererEvent->sceneId.getValue(), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneExpired:
                rendererEventHandler.sceneUpdateLatencyExceeded(rendererEvent->sceneId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneRecoveredFromExpiration:
                rendererEventHandler.sceneUpdateLatencyBackBelowLimit(rendererEvent->sceneId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataLinked:
                rendererEventHandler.dataLinked(rendererEvent->providerSceneId.getValue(), rendererEvent->providerdataId.getValue(), rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataLinkFailed:
                rendererEventHandler.dataLinked(rendererEvent->providerSceneId.getValue(), rendererEvent->providerdataId.getValue(), rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinked:
                rendererEventHandler.offscreenBufferLinkedToSceneData(rendererEvent->offscreenBuffer.asMemoryHandle(), rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinkFailed:
                rendererEventHandler.offscreenBufferLinkedToSceneData(rendererEvent->offscreenBuffer.asMemoryHandle(), rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinked:
                rendererEventHandler.dataUnlinked(rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange:
                rendererEventHandler.dataUnlinked(rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkFailed:
                rendererEventHandler.dataUnlinked(rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_DisplayCreated:
                rendererEventHandler.displayCreated(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_DisplayCreateFailed:
                rendererEventHandler.displayCreated(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_DisplayDestroyed:
                rendererEventHandler.displayDestroyed(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_DisplayDestroyFailed:
                rendererEventHandler.displayDestroyed(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_ReadPixelsFromFramebuffer:
            {
                const ramses_internal::UInt8Vector& pixelData = rendererEvent->pixelData;
                assert(!pixelData.empty());
                rendererEventHandler.framebufferPixelsRead(&pixelData.front(), static_cast<uint32_t>(pixelData.size()), rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_OK);
                break;
            }
            case ramses_internal::ERendererEventType_ReadPixelsFromFramebufferFailed:
                rendererEventHandler.framebufferPixelsRead(NULL, 0u, rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_WarpingDataUpdated:
                rendererEventHandler.warpingMeshDataUpdated(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_WarpingDataUpdateFailed:
                rendererEventHandler.warpingMeshDataUpdated(rendererEvent->displayHandle.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferCreated:
                rendererEventHandler.offscreenBufferCreated(rendererEvent->displayHandle.asMemoryHandle(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferCreateFailed:
                rendererEventHandler.offscreenBufferCreated(rendererEvent->displayHandle.asMemoryHandle(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferDestroyed:
                rendererEventHandler.offscreenBufferDestroyed(rendererEvent->displayHandle.asMemoryHandle(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_OffscreenBufferDestroyFailed:
                rendererEventHandler.offscreenBufferDestroyed(rendererEvent->displayHandle.asMemoryHandle(), rendererEvent->offscreenBuffer.asMemoryHandle(), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderCreated:
                rendererEventHandler.dataProviderCreated(rendererEvent->providerSceneId.getValue(), rendererEvent->providerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderDestroyed:
                rendererEventHandler.dataProviderDestroyed(rendererEvent->providerSceneId.getValue(), rendererEvent->providerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerCreated:
                rendererEventHandler.dataConsumerCreated(rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerDestroyed:
                rendererEventHandler.dataConsumerDestroyed(rendererEvent->consumerSceneId.getValue(), rendererEvent->consumerdataId.getValue());
                break;
            case ramses_internal::ERendererEventType_WindowClosed:
                rendererEventHandler.windowClosed(rendererEvent->displayHandle.asMemoryHandle());
                break;
            case ramses_internal::ERendererEventType_WindowKeyEvent:
                rendererEventHandler.keyEvent(rendererEvent->displayHandle.asMemoryHandle(),
                    RamsesRendererUtils::GetKeyEvent(rendererEvent->keyEvent.type),
                    rendererEvent->keyEvent.modifier, RamsesRendererUtils::GetKeyCode(rendererEvent->keyEvent.keyCode));
                break;
            case ramses_internal::ERendererEventType_WindowMouseEvent:
                rendererEventHandler.mouseEvent(rendererEvent->displayHandle.asMemoryHandle(),
                    RamsesRendererUtils::GetMouseEvent(rendererEvent->mouseEvent.type),
                    rendererEvent->mouseEvent.pos.x, rendererEvent->mouseEvent.pos.y);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceAvailable:
                rendererEventHandler.streamAvailabilityChanged(ramses::streamSource_t(rendererEvent->streamSourceId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceUnavailable:
                rendererEventHandler.streamAvailabilityChanged(ramses::streamSource_t(rendererEvent->streamSourceId.getValue()), false);
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
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.logRendererInfo(ramses_internal::ERendererLogTopic_All, true, ramses_internal::NodeHandle::Invalid());
        return StatusOK;
    }

    ramses::status_t RamsesRendererImpl::startThread()
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
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

    ramses::status_t RamsesRendererImpl::stopThread()
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
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
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        return m_rendererLoopThreadController.isRendering();
    }

    bool RamsesRendererImpl::isThreaded() const
    {
        return m_rendererLoopThreadType == ERendererLoopThreadType_InRendererOwnThread;
    }

    ramses::status_t RamsesRendererImpl::setMaximumFramerate(float maximumFramerate)
    {
        if (maximumFramerate <= 0.0f)
        {
            return addErrorEntry("RamsesRenderer::setMaximumFramerate must specify a positive maximumFramerate!");
        }

        ramses_internal::PlatformLightweightGuard guard(m_lock);
        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
        {
            return addErrorEntry("RamsesRenderer::setMaximumFramerate Can not call setMaximumFramerate if doOneLoop is called before because it can only control framerate for rendering thread!");
        }

        m_rendererLoopThreadController.setMaximumFramerate(maximumFramerate);
        return StatusOK;
    }

    float RamsesRendererImpl::getMaximumFramerate() const
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        return m_rendererLoopThreadController.getMaximumFramerate();
    }

    ramses::status_t RamsesRendererImpl::setLoopMode(ELoopMode loopMode)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);

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
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        return m_loopMode == ramses_internal::ELoopMode_UpdateAndRender? ramses::ELoopMode_UpdateAndRender : ramses::ELoopMode_UpdateOnly;
    }

    ramses::status_t RamsesRendererImpl::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.setFrameTimerLimits(limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
        return StatusOK;
    }

    ramses::status_t RamsesRendererImpl::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.setForceApplyPendingFlushesLimit(forceApplyFlushLimit);
        m_pendingRendererCommands.setForceUnsubscribeLimits(forceUnsubscribeSceneLimit);
        return StatusOK;
    }

    ramses::status_t RamsesRendererImpl::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        ramses_internal::PlatformLightweightGuard guard(m_lock);
        m_pendingRendererCommands.setSkippingOfUnmodifiedBuffers(enable);
        return StatusOK;
    }
}
