//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererResourceManager.h"
#include "internal/RendererLib/IResourceUploader.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositingManager.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/Components/ManagedResource.h"
#include "internal/SceneGraph/Resource/ResourceInfo.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include "internal/Core/Utils/TextureMathUtils.h"
#include "internal/Core/Utils/LogMacros.h"
#include <memory>

namespace ramses::internal
{
    RendererResourceManager::RendererResourceManager(
        IRenderBackend& renderBackend,
        std::unique_ptr<IResourceUploader> resourceUploader,
        AsyncEffectUploader& asyncEffectUploader,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        const DisplayConfig& displayConfig,
        const FrameTimer& frameTimer,
        RendererStatistics& stats)
        : m_renderBackend(renderBackend)
        , m_embeddedCompositingManager(embeddedCompositingManager)
        , m_resourceUploadingManager(m_resourceRegistry, std::move(resourceUploader), renderBackend, asyncEffectUploader, displayConfig, frameTimer, stats)
        , m_stats(stats)
    {
    }

    RendererResourceManager::~RendererResourceManager()
    {
        assert(m_sceneResourceRegistryMap.size() == 0u);

        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager::~RendererResourceManager Destroying offscreen buffers");
        for (OffscreenBufferHandle handle{ 0u }; handle < m_offscreenBuffers.getTotalCount(); ++handle)
        {
            if (m_offscreenBuffers.isAllocated(handle))
                unloadOffscreenBuffer(handle);
        }
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager::~RendererResourceManager Destroying stream buffers");
        for (StreamBufferHandle handle{ 0u }; handle < m_streamBuffers.getTotalCount(); ++handle)
        {
            if (m_streamBuffers.isAllocated(handle))
                unloadStreamBuffer(handle);
        }

        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager::~RendererResourceManager Destroying external buffers");
        for (ExternalBufferHandle handle{ 0u }; handle < m_externalBuffers.getTotalCount(); ++handle)
        {
            if (m_externalBuffers.isAllocated(handle))
                unloadExternalBuffer(handle);
        }

        for (const auto& resDesc [[maybe_unused]] : m_resourceRegistry.getAllResourceDescriptors())
        {
            assert(resDesc.value.sceneUsage.empty());
        }
    }

    void RendererResourceManager::referenceResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
    {
        for (const auto& resHash : resources)
        {
            if (!m_resourceRegistry.containsResource(resHash))
                m_resourceRegistry.registerResource(resHash);

            m_resourceRegistry.addResourceRef(resHash, sceneId);
        }
    }

    void RendererResourceManager::unreferenceResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
    {
        for (const auto& resHash : resources)
            m_resourceRegistry.removeResourceRef(resHash, sceneId);
    }

    RendererSceneResourceRegistry& RendererResourceManager::getSceneResourceRegistry(SceneId sceneId)
    {
        if (!m_sceneResourceRegistryMap.contains(sceneId))
        {
            m_sceneResourceRegistryMap.put(sceneId, RendererSceneResourceRegistry());
        }
        return *m_sceneResourceRegistryMap.get(sceneId);
    }

    void RendererResourceManager::clearRenderTarget(DeviceResourceHandle handle)
    {
        // TODO Violin/Mohamed: this code could be optimized if needed... No need to have two additional FBO activations just to set their clear state
        IDevice& device = m_renderBackend.getDevice();
        device.activateRenderTarget(handle);
        device.colorMask(true, true, true, true);
        device.clearColor({ 0.f, 0.f, 0.f, 1.f });
        device.depthWrite(EDepthWrite::Enabled);
        device.scissorTest(EScissorTest::Disabled, {});
        device.clear(EClearFlag::All);
    }

    void RendererResourceManager::unloadAllSceneResourcesForScene(SceneId sceneId)
    {
        if (m_sceneResourceRegistryMap.contains(sceneId))
        {
            RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

            RenderTargetHandleVector renderTargets;
            sceneResources.getAllRenderTargets(renderTargets);
            for (const auto& rt : renderTargets)
                unloadRenderTarget(rt, sceneId);

            BlitPassHandleVector blitPasses;
            sceneResources.getAllBlitPasses(blitPasses);
            for (const auto& bp : blitPasses)
                unloadBlitPassRenderTargets(bp, sceneId);

            RenderBufferHandleVector renderBuffers;
            sceneResources.getAllRenderBuffers(renderBuffers);
            for(const auto& rb : renderBuffers)
                unloadRenderTargetBuffer(rb, sceneId);

            RenderableVector vertexArrayRenderables;
            sceneResources.getAllVertexArrayRenderables(vertexArrayRenderables);
            for (const auto r : vertexArrayRenderables)
                unloadVertexArray(r, sceneId);

            DataBufferHandleVector dataBuffers;
            sceneResources.getAllDataBuffers(dataBuffers);
            for (const auto db : dataBuffers)
                unloadDataBuffer(db, sceneId);

            TextureBufferHandleVector textureBuffers;
            sceneResources.getAllTextureBuffers(textureBuffers);
            for (const auto tb : textureBuffers)
                unloadTextureBuffer(tb, sceneId);

            m_sceneResourceRegistryMap.remove(sceneId);
        }
    }

    void RendererResourceManager::unreferenceAllResourcesForScene(SceneId sceneId)
    {
        if (m_resourceRegistry.getResourcesInUseByScene(sceneId))
        {
            // make copy as it will be modified while iterating
            const ResourceContentHashVector usedResources = *m_resourceRegistry.getResourcesInUseByScene(sceneId);
            for (const auto& res : usedResources)
            {
                while (m_resourceRegistry.containsResource(res) && contains_c(m_resourceRegistry.getResourceDescriptor(res).sceneUsage, sceneId))
                    m_resourceRegistry.removeResourceRef(res, sceneId);
            }
        }
    }

    const ResourceContentHashVector* RendererResourceManager::getResourcesInUseByScene(SceneId sceneId) const
    {
        return m_resourceRegistry.getResourcesInUseByScene(sceneId);
    }

    void RendererResourceManager::provideResourceData(const ManagedResource& mr)
    {
        const ResourceContentHash resHash = mr->getHash();
        assert(m_resourceRegistry.containsResource(resHash));

        if (m_resourceRegistry.getResourceStatus(resHash) == EResourceStatus::Registered)
            m_resourceRegistry.setResourceData(resHash, mr);
    }

    bool RendererResourceManager::hasResourcesToBeUploaded() const
    {
        return m_resourceUploadingManager.hasAnythingToUpload();
    }

    void RendererResourceManager::uploadAndUnloadPendingResources()
    {
        m_resourceUploadingManager.uploadAndUnloadPendingResources();
    }

    EResourceStatus RendererResourceManager::getResourceStatus(const ResourceContentHash& hash) const
    {
        return m_resourceRegistry.getResourceStatus(hash);
    }

    EResourceType RendererResourceManager::getResourceType(const ResourceContentHash& hash) const
    {
        return m_resourceRegistry.getResourceDescriptor(hash).type;
    }

    DeviceResourceHandle RendererResourceManager::getResourceDeviceHandle(const ResourceContentHash& hash) const
    {
        return m_resourceRegistry.getResourceDescriptor(hash).deviceHandle;
    }

    DeviceResourceHandle RendererResourceManager::getRenderTargetDeviceHandle(RenderTargetHandle handle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getRenderTargetDeviceHandle(handle);
    }

    DeviceResourceHandle RendererResourceManager::getRenderTargetBufferDeviceHandle(RenderBufferHandle bufferHandle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getRenderBufferDeviceHandle(bufferHandle);
    }

    void RendererResourceManager::getBlitPassRenderTargetsDeviceHandle(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle& srcRT, DeviceResourceHandle& dstRT) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        sceneResources.getBlitPassDeviceHandles(blitPassHandle, srcRT, dstRT);
    }

    DeviceResourceHandle RendererResourceManager::getOffscreenBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const
    {
        // Always give the handle to the first render target, in the case of double-buffering the device will do the resolving of
        // which is the current "front" render target which can be read
        return (m_offscreenBuffers.isAllocated(bufferHandle) ? m_offscreenBuffers.getMemory(bufferHandle)->m_renderTargetHandle[0] : DeviceResourceHandle::Invalid());
    }

    DeviceResourceHandle RendererResourceManager::getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const
    {
        assert(m_offscreenBuffers.isAllocated(bufferHandle));
        // Always give the handle to the first color buffer, in the case of double-buffering the device will do the resolving of
        // which is the current "front" color buffer which can be read
        return m_offscreenBuffers.getMemory(bufferHandle)->m_colorBufferHandle[0];
    }

    int RendererResourceManager::getDmaOffscreenBufferFD(OffscreenBufferHandle bufferHandle) const
    {
        assert(m_offscreenBuffers.isAllocated(bufferHandle));
        const auto& obDescriptor = *m_offscreenBuffers.getMemory(bufferHandle);
        return m_renderBackend.getDevice().getDmaRenderBufferFD(obDescriptor.m_colorBufferHandle[0u]);
    }

    uint32_t RendererResourceManager::getDmaOffscreenBufferStride(OffscreenBufferHandle bufferHandle) const
    {
        assert(m_offscreenBuffers.isAllocated(bufferHandle));
        const auto& obDescriptor = *m_offscreenBuffers.getMemory(bufferHandle);
        return m_renderBackend.getDevice().getDmaRenderBufferStride(obDescriptor.m_colorBufferHandle[0u]);
    }

    OffscreenBufferHandle RendererResourceManager::getOffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle) const
    {
        for (OffscreenBufferHandle handle(0u); handle < m_offscreenBuffers.getTotalCount(); ++handle)
        {
            if (m_offscreenBuffers.isAllocated(handle) && m_offscreenBuffers.getMemory(handle)->m_renderTargetHandle[0] == bufferDeviceHandle)
                return handle;
        }

        return OffscreenBufferHandle::Invalid();
    }

    DeviceResourceHandle RendererResourceManager::getStreamBufferDeviceHandle(StreamBufferHandle bufferHandle) const
    {
        if (!m_streamBuffers.isAllocated(bufferHandle))
            return DeviceResourceHandle::Invalid();

        const auto surfaceId = *m_streamBuffers.getMemory(bufferHandle);
        return m_embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(surfaceId);
    }

    DeviceResourceHandle RendererResourceManager::getExternalBufferDeviceHandle(ExternalBufferHandle bufferHandle) const
    {
        if(!m_externalBuffers.isAllocated(bufferHandle))
            return {};

        return *m_externalBuffers.getMemory(bufferHandle);
    }

    DeviceResourceHandle RendererResourceManager::getEmptyExternalBufferDeviceHandle() const
    {
        return m_renderBackend.getDevice().getEmptyExternalTexture();
    }

    uint32_t RendererResourceManager::getExternalBufferGlId(ExternalBufferHandle bufferHandle) const
    {
        if (!m_externalBuffers.isAllocated(bufferHandle))
            return 0u;

        const auto deviceHandle = *m_externalBuffers.getMemory(bufferHandle);
        return m_renderBackend.getDevice().getTextureAddress(deviceHandle);
    }

    const StreamUsage& RendererResourceManager::getStreamUsage(WaylandIviSurfaceId source) const
    {
        assert(m_streamUsages.count(source));
        return m_streamUsages.find(source)->second;
    }

    const RendererResourceRegistry& RendererResourceManager::getRendererResourceRegistry() const
    {
        return m_resourceRegistry;
    }

    void RendererResourceManager::uploadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId, const RenderBuffer& renderBuffer)
    {
        const uint32_t memSize = renderBuffer.width * renderBuffer.height * GetTexelSizeFromFormat(renderBuffer.format) * std::max(1u, renderBuffer.sampleCount);
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadRenderTargetBuffer sceneId={} handle={} {}x{} {} samples={} estimatedSize={}KB", sceneId, renderBufferHandle,
            renderBuffer.width, renderBuffer.height, EnumToString(renderBuffer.format), renderBuffer.sampleCount, memSize/1024);

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        IDevice& device = m_renderBackend.getDevice();
        const DeviceResourceHandle deviceHandle = device.uploadRenderBuffer(renderBuffer.width, renderBuffer.height, renderBuffer.format, renderBuffer.accessMode, renderBuffer.sampleCount);
        assert(deviceHandle.isValid());
        if (!deviceHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::uploadRenderTargetBuffer failed to create render buffer, this is fatal...");
            std::ignore = device.isDeviceStatusHealthy();
        }

        sceneResources.addRenderBuffer(renderBufferHandle, deviceHandle, memSize, ERenderBufferAccessMode::WriteOnly == renderBuffer.accessMode);
        m_stats.sceneResourceUploaded(sceneId, memSize);
    }

    void RendererResourceManager::unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadRenderTargetBuffer sceneId={} handle={}", sceneId, renderBufferHandle);

        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        IDevice& device = m_renderBackend.getDevice();
        device.deleteRenderBuffer(sceneResources.getRenderBufferDeviceHandle(renderBufferHandle));
        sceneResources.removeRenderBuffer(renderBufferHandle);
    }

    void RendererResourceManager::uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId)
    {
        LOG_INFO_PF(CONTEXT_RENDERER, ([&](auto& out) {
            fmt::format_to(std::back_inserter(out), "RendererResourceManager::uploadRenderTarget sceneId={} handle={} renderBufferHandles:", sceneId, renderTarget);
            for (const auto rb : rtBufferHandles)
                fmt::format_to(std::back_inserter(out), " {}", rb);
        }));

        assert(!rtBufferHandles.empty());
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        DeviceHandleVector rtBufferDeviceHandles;
        rtBufferDeviceHandles.reserve(rtBufferHandles.size());
        for(const auto& rb : rtBufferHandles)
        {
            const DeviceResourceHandle rbDeviceHandle = sceneResources.getRenderBufferDeviceHandle(rb);
            assert(rbDeviceHandle.isValid());
            rtBufferDeviceHandles.push_back(rbDeviceHandle);
        }

        IDevice& device = m_renderBackend.getDevice();
        const DeviceResourceHandle rtDeviceHandle = device.uploadRenderTarget(rtBufferDeviceHandles);

        sceneResources.addRenderTarget(renderTarget, rtDeviceHandle);
    }

    void RendererResourceManager::unloadRenderTarget(RenderTargetHandle renderTarget, SceneId sceneId)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadRenderTarget sceneId={} handle={}", sceneId, renderTarget);

        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        const DeviceResourceHandle rtDeviceHandle = sceneResources.getRenderTargetDeviceHandle(renderTarget);
        IDevice& device = m_renderBackend.getDevice();
        device.deleteRenderTarget(rtDeviceHandle);
        sceneResources.removeRenderTarget(renderTarget);
    }

    void RendererResourceManager::uploadDmaOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers)
    {
        assert(!m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();

        m_offscreenBuffers.allocate(bufferHandle);
        OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);
        offscreenBufferDesc.isDmaBuffer = true;

        offscreenBufferDesc.m_colorBufferHandle[0] = device.uploadDmaRenderBuffer(width, height, dmaBufferFourccFormat, dmaBufferUsageFlags, dmaBufferModifiers);

        offscreenBufferDesc.m_estimatedVRAMUsage = width * height * GetTexelSizeFromFormat(EPixelStorageFormat::RGBA8);
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadDmaOffscreenBuffer handle={} {}x{} fourccFormat={} usage={} modifier={} estimatedSize(RGBA8)={}KB", bufferHandle,
            width, height, dmaBufferFourccFormat.getValue(), dmaBufferUsageFlags.getValue(), dmaBufferModifiers.getValue(), offscreenBufferDesc.m_estimatedVRAMUsage / 1024);

        offscreenBufferDesc.m_renderTargetHandle[0] = device.uploadRenderTarget({ offscreenBufferDesc.m_colorBufferHandle[0] });

        clearRenderTarget(offscreenBufferDesc.m_renderTargetHandle[0]);
    }

    void RendererResourceManager::uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, EDepthBufferType depthStencilBufferType)
    {
        assert(!m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();

        m_offscreenBuffers.allocate(bufferHandle);
        OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);
        offscreenBufferDesc.isDmaBuffer = false;

        offscreenBufferDesc.m_colorBufferHandle[0] = device.uploadRenderBuffer(width, height, EPixelStorageFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, sampleCount);

        uint32_t texelSize = GetTexelSizeFromFormat(EPixelStorageFormat::RGBA8) * (isDoubleBuffered ? 2u : 1u); // only color buffer is double
        switch (depthStencilBufferType)
        {
        case EDepthBufferType::None:
            offscreenBufferDesc.m_depthBufferHandle = {};
            break;
        case EDepthBufferType::Depth:
            offscreenBufferDesc.m_depthBufferHandle =
                device.uploadRenderBuffer(width, height, EPixelStorageFormat::Depth32, ERenderBufferAccessMode::WriteOnly, sampleCount);
            texelSize += GetTexelSizeFromFormat(EPixelStorageFormat::Depth32);
            break;
        case EDepthBufferType::DepthStencil:
            offscreenBufferDesc.m_depthBufferHandle =
                device.uploadRenderBuffer(width, height, EPixelStorageFormat::Depth24_Stencil8, ERenderBufferAccessMode::WriteOnly, sampleCount);
            texelSize += GetTexelSizeFromFormat(EPixelStorageFormat::Depth24_Stencil8);
            break;
        }

        offscreenBufferDesc.m_estimatedVRAMUsage = width * height * std::max(1u, sampleCount) * texelSize;
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadOffscreenBuffer handle={} {}x{} samples={} interruptible={} depthType={} estimatedSize={}KB", bufferHandle,
            width, height, sampleCount, isDoubleBuffered, depthStencilBufferType, offscreenBufferDesc.m_estimatedVRAMUsage / 1024);

        DeviceHandleVector bufferDeviceHandles;
        bufferDeviceHandles.push_back(offscreenBufferDesc.m_colorBufferHandle[0]);
        if(offscreenBufferDesc.m_depthBufferHandle.isValid())
            bufferDeviceHandles.push_back(offscreenBufferDesc.m_depthBufferHandle);
        offscreenBufferDesc.m_renderTargetHandle[0] = device.uploadRenderTarget(bufferDeviceHandles);

        if (isDoubleBuffered)
        {
            offscreenBufferDesc.m_colorBufferHandle[1] = device.uploadRenderBuffer(width, height, EPixelStorageFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, sampleCount);

            DeviceHandleVector bufferDeviceHandles2;
            bufferDeviceHandles2.push_back(offscreenBufferDesc.m_colorBufferHandle[1]);
            if(offscreenBufferDesc.m_depthBufferHandle.isValid())
                bufferDeviceHandles2.push_back(offscreenBufferDesc.m_depthBufferHandle);
            offscreenBufferDesc.m_renderTargetHandle[1] = device.uploadRenderTarget(bufferDeviceHandles2);
        }

        // Initial clear of the buffers
        clearRenderTarget(offscreenBufferDesc.m_renderTargetHandle[0]);

        if (isDoubleBuffered)
        {
            clearRenderTarget(offscreenBufferDesc.m_renderTargetHandle[1]);
            device.pairRenderTargetsForDoubleBuffering(offscreenBufferDesc.m_renderTargetHandle, offscreenBufferDesc.m_colorBufferHandle);
        }
    }

    void RendererResourceManager::unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadOffscreenBuffer handle={}", bufferHandle);

        assert(m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();
        const OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);

        device.deleteRenderTarget(offscreenBufferDesc.m_renderTargetHandle[0]);

        if (offscreenBufferDesc.isDmaBuffer)
        {
            device.destroyDmaRenderBuffer(offscreenBufferDesc.m_colorBufferHandle[0]);
        }
        else
        {
            if (offscreenBufferDesc.m_renderTargetHandle[1].isValid())
            {
                device.deleteRenderTarget(offscreenBufferDesc.m_renderTargetHandle[1]);
                device.unpairRenderTargets(offscreenBufferDesc.m_renderTargetHandle[0]);
            }
            device.deleteRenderBuffer(offscreenBufferDesc.m_colorBufferHandle[0]);

            if(offscreenBufferDesc.m_depthBufferHandle.isValid())
                device.deleteRenderBuffer(offscreenBufferDesc.m_depthBufferHandle);

            if (offscreenBufferDesc.m_colorBufferHandle[1].isValid())
                device.deleteRenderBuffer(offscreenBufferDesc.m_colorBufferHandle[1]);
        }

        m_offscreenBuffers.release(bufferHandle);
    }

    void RendererResourceManager::uploadStreamBuffer(StreamBufferHandle bufferHandle, WaylandIviSurfaceId source)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadStreamBuffer handle={} source={}", bufferHandle, source);

        assert(!m_streamBuffers.isAllocated(bufferHandle));
        m_streamBuffers.allocate(bufferHandle);
        *m_streamBuffers.getMemory(bufferHandle) = source;

        m_embeddedCompositingManager.refStream(source);

        assert(!contains_c(m_streamUsages[source], bufferHandle));
        m_streamUsages[source].push_back(bufferHandle);
    }

    void RendererResourceManager::unloadStreamBuffer(StreamBufferHandle bufferHandle)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadStreamBuffer handle={}", bufferHandle);

        assert(m_streamBuffers.isAllocated(bufferHandle));
        const auto source = *m_streamBuffers.getMemory(bufferHandle);
        m_streamBuffers.release(bufferHandle);

        m_embeddedCompositingManager.unrefStream(source);

        auto& streamUsage = m_streamUsages[source];
        assert(contains_c(streamUsage, bufferHandle));
        streamUsage.erase(find_c(streamUsage, bufferHandle));
    }


    void RendererResourceManager::uploadExternalBuffer(ExternalBufferHandle bufferHandle)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadExternalBuffer handle={}", bufferHandle);

        IDevice& device = m_renderBackend.getDevice();

        if (!device.isExternalTextureExtensionSupported())
        {
            LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadExternalBuffer failed uploading handle={} because extension not supported on device", bufferHandle);
            return;
        }

        auto deviceHandle = device.allocateExternalTexture();
        assert(!m_externalBuffers.isAllocated(bufferHandle));
        m_externalBuffers.allocate(bufferHandle);
        *m_externalBuffers.getMemory(bufferHandle) = deviceHandle;
    }

    void RendererResourceManager::unloadExternalBuffer(ExternalBufferHandle bufferHandle)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadExternalBuffer handle={}", bufferHandle);

        assert(m_externalBuffers.isAllocated(bufferHandle));
        const auto deviceHandle = *m_externalBuffers.getMemory(bufferHandle);
        m_externalBuffers.release(bufferHandle);

        m_renderBackend.getDevice().deleteTexture(deviceHandle);
    }

    void RendererResourceManager::uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadBlitPassRenderTargets sceneId={} handle={} srcBufferHandle={} dstBufferHandle={}", sceneId, blitPass, sourceRenderBuffer, destinationRenderBuffer);

        assert(blitPass.isValid());
        assert(sourceRenderBuffer.isValid());
        assert(destinationRenderBuffer.isValid());
        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        const DeviceResourceHandle sourceRenderBufferDeviceHandle = sceneResources.getRenderBufferDeviceHandle(sourceRenderBuffer);
        const DeviceResourceHandle destinationRenderBufferDeviceHandle = sceneResources.getRenderBufferDeviceHandle(destinationRenderBuffer);
        assert(sourceRenderBufferDeviceHandle.isValid() && destinationRenderBufferDeviceHandle.isValid());

        IDevice& device = m_renderBackend.getDevice();
        DeviceHandleVector rbDeviceHandles;
        rbDeviceHandles.push_back(sourceRenderBufferDeviceHandle);
        const DeviceResourceHandle blitRtSource = device.uploadRenderTarget(rbDeviceHandles);
        rbDeviceHandles.clear();
        rbDeviceHandles.push_back(destinationRenderBufferDeviceHandle);
        const DeviceResourceHandle blitRtDest = device.uploadRenderTarget(rbDeviceHandles);
        assert(blitRtSource.isValid() && blitRtDest.isValid());
        if (!blitRtSource.isValid() || !blitRtDest.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::uploadBlitPassRenderTargets failed to create blit render target(s), this is fatal...");
            std::ignore = device.isDeviceStatusHealthy();
        }

        sceneResources.addBlitPass(blitPass, blitRtSource, blitRtDest);
    }

    void RendererResourceManager::unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::unloadBlitPassRenderTargets sceneId={} handle={}", sceneId, blitPass);

        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        DeviceResourceHandle srcRTDeviceHandle;
        DeviceResourceHandle dstRTDeviceHandle;
        sceneResources.getBlitPassDeviceHandles(blitPass, srcRTDeviceHandle, dstRTDeviceHandle);
        sceneResources.removeBlitPass(blitPass);

        IDevice& device = m_renderBackend.getDevice();
        device.deleteRenderTarget(srcRTDeviceHandle);
        device.deleteRenderTarget(dstRTDeviceHandle);
    }

    void RendererResourceManager::uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, uint32_t dataSizeInBytes, SceneId sceneId)
    {
        assert(dataBufferHandle.isValid());
        assert(EDataType::Invalid != dataType);

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle;
        switch (dataBufferType)
        {
        case EDataBufferType::IndexBuffer:
            deviceHandle = device.allocateIndexBuffer(dataType, dataSizeInBytes);
            break;
        case EDataBufferType::VertexBuffer:
            deviceHandle = device.allocateVertexBuffer(dataSizeInBytes);
            break;
        default:
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::uploadDataBuffer: can not upload data buffer with invalid type!");
            assert(false);
        }

        assert(deviceHandle.isValid());
        if (!deviceHandle.isValid())
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "RendererResourceManager::uploadDataBuffer sceneId={} handle={} dataType={} size={} failed to allocate data buffer, this is fatal...",
                sceneId, dataBufferHandle, EnumToString(dataType), dataSizeInBytes);
            std::ignore = device.isDeviceStatusHealthy();
        }

        sceneResources.addDataBuffer(dataBufferHandle, deviceHandle, dataBufferType, dataSizeInBytes);
        m_stats.sceneResourceUploaded(sceneId, dataSizeInBytes);
    }

    void RendererResourceManager::unloadDataBuffer(DataBufferHandle dataBufferHandle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getDataBufferDeviceHandle(dataBufferHandle);
        assert(deviceHandle.isValid());
        const EDataBufferType dataBufferType = sceneResources.getDataBufferType(dataBufferHandle);

        IDevice& device = m_renderBackend.getDevice();
        switch (dataBufferType)
        {
        case EDataBufferType::IndexBuffer:
            device.deleteIndexBuffer(deviceHandle);
            break;
        case EDataBufferType::VertexBuffer:
            device.deleteVertexBuffer(deviceHandle);
            break;
        default:
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::unloadDataBuffer: can not unload data buffer with invalid type!");
            assert(false);
        }

        sceneResources.removeDataBuffer(dataBufferHandle);
    }

    void RendererResourceManager::updateDataBuffer(DataBufferHandle handle, uint32_t dataSizeInBytes, const std::byte* data, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getDataBufferDeviceHandle(handle);
        assert(deviceHandle.isValid());
        const EDataBufferType dataBufferType = sceneResources.getDataBufferType(handle);

        IDevice& device = m_renderBackend.getDevice();
        switch (dataBufferType)
        {
        case EDataBufferType::IndexBuffer:
            device.uploadIndexBufferData(deviceHandle, data, dataSizeInBytes);
            break;
        case EDataBufferType::VertexBuffer:
            device.uploadVertexBufferData(deviceHandle, data, dataSizeInBytes);
            break;
        default:
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::updateDataBuffer: can not updata data buffer with invalid type!");
            assert(false);
        }
        m_stats.sceneResourceUploaded(sceneId, dataSizeInBytes);
    }

    DeviceResourceHandle RendererResourceManager::getDataBufferDeviceHandle(DataBufferHandle dataBufferHandle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getDataBufferDeviceHandle(dataBufferHandle);
    }

    void RendererResourceManager::uploadTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t width, uint32_t height, EPixelStorageFormat textureFormat, uint32_t mipLevelCount, SceneId sceneId)
    {
        assert(textureBufferHandle.isValid());
        assert(EPixelStorageFormat::Invalid != textureFormat);
        assert(!IsFormatCompressed(textureFormat)); //can not be a compressed format

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        const uint32_t totalSizeInBytes = TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(textureFormat), width, height, 1u, mipLevelCount);
        LOG_INFO_P(CONTEXT_RENDERER, "RendererResourceManager::uploadTextureBuffer sceneId={} handle={} {}x{} format={} mips={} estimatedSize={}KB", sceneId, textureBufferHandle,
            width, height, EnumToString(textureFormat), mipLevelCount, totalSizeInBytes / 1024);

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle = device.allocateTexture2D(width, height, textureFormat, DefaultTextureSwizzleArray, mipLevelCount, totalSizeInBytes);
        assert(deviceHandle.isValid());
        if (!deviceHandle.isValid())
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "RendererResourceManager::uploadTextureBuffer failed to allocate texture buffer, this is fatal...");
            std::ignore = device.isDeviceStatusHealthy();
        }

        sceneResources.addTextureBuffer(textureBufferHandle, deviceHandle, textureFormat, totalSizeInBytes);
        m_stats.sceneResourceUploaded(sceneId, totalSizeInBytes);
    }

    void RendererResourceManager::unloadTextureBuffer(TextureBufferHandle textureBufferHandle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getTextureBufferDeviceHandle(textureBufferHandle);
        assert(deviceHandle.isValid());
        IDevice& device = m_renderBackend.getDevice();
        device.deleteTexture(deviceHandle);

        sceneResources.removeTextureBuffer(textureBufferHandle);
    }

    void RendererResourceManager::updateTextureBuffer(TextureBufferHandle textureBufferHandle, uint32_t mipLevel, const Quad& area, uint32_t stride, const std::byte* data, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getTextureBufferDeviceHandle(textureBufferHandle);
        assert(deviceHandle.isValid());

        IDevice& device = m_renderBackend.getDevice();
        device.bindTexture(deviceHandle);
        device.uploadTextureData(deviceHandle, mipLevel, area.x, area.y, 0u, area.width, area.height, 1u, data, 0u, stride);

        const uint32_t updateDataSizeInBytes = TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(sceneResources.getTextureBufferFormat(textureBufferHandle)), area.width, area.height, 1u, 1u);
        m_stats.sceneResourceUploaded(sceneId, updateDataSizeInBytes);
    }

    DeviceResourceHandle RendererResourceManager::getTextureBufferDeviceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getTextureBufferDeviceHandle(textureBufferHandle);
    }

    void RendererResourceManager::uploadVertexArray(RenderableHandle renderableHandle, const VertexArrayInfo& vertexArrayInfo, SceneId sceneId)
    {
        IDevice& device = m_renderBackend.getDevice();
        const auto deviceHandle = device.allocateVertexArray(vertexArrayInfo);
        assert(deviceHandle.isValid());
        if (!deviceHandle.isValid())
        {
            LOG_ERROR_P(CONTEXT_RENDERER, "RendererResourceManager::uploadVertexArray sceneId={} renderableHandle={} failed to allocate vertex array, this is fatal...",
                sceneId, renderableHandle);
            std::ignore = device.isDeviceStatusHealthy();
        }

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        sceneResources.addVertexArray(renderableHandle, deviceHandle);
    }

    void RendererResourceManager::unloadVertexArray(RenderableHandle renderableHandle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        const auto deviceHandle = sceneResources.getVertexArrayDeviceHandle(renderableHandle);
        m_renderBackend.getDevice().deleteVertexArray(deviceHandle);
        sceneResources.removeVertexArray(renderableHandle);
    }

    DeviceResourceHandle RendererResourceManager::getVertexArrayDeviceHandle(RenderableHandle renderableHandle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getVertexArrayDeviceHandle(renderableHandle);
    }
}
