//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/RendererResourceManagerUtils.h"
#include "RendererLib/IResourceUploader.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/IRendererResourceCache.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/EDataBufferType.h"
#include "Components/ManagedResource.h"
#include "Resource/ResourceInfo.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/TextureMathUtils.h"
#include "Math3d/Vector4.h"
#include <memory>

namespace ramses_internal
{
    RendererResourceManager::RendererResourceManager(
        IRenderBackend& renderBackend,
        std::unique_ptr<IResourceUploader> resourceUploader,
        AsyncEffectUploader& asyncEffectUploader,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        Bool keepEffects,
        const FrameTimer& frameTimer,
        RendererStatistics& stats,
        UInt64 gpuCacheSize)
        : m_renderBackend(renderBackend)
        , m_embeddedCompositingManager(embeddedCompositingManager)
        , m_resourceUploadingManager(m_resourceRegistry, std::move(resourceUploader), renderBackend, asyncEffectUploader, keepEffects, frameTimer, stats, gpuCacheSize)
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

        for (const auto& resDesc : m_resourceRegistry.getAllResourceDescriptors())
        {
            UNUSED(resDesc);
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

    void RendererResourceManager::unloadAllSceneResourcesForScene(SceneId sceneId)
    {
        if (m_sceneResourceRegistryMap.contains(sceneId))
        {
            RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

            RenderBufferHandleVector renderBuffers;
            sceneResources.getAllRenderBuffers(renderBuffers);
            for(const auto& rb : renderBuffers)
            {
                unloadRenderTargetBuffer(rb, sceneId);
            }

            RenderTargetHandleVector renderTargets;
            sceneResources.getAllRenderTargets(renderTargets);
            for (const auto& rt : renderTargets)
            {
                unloadRenderTarget(rt, sceneId);
            }

            BlitPassHandleVector blitPasses;
            sceneResources.getAllBlitPasses(blitPasses);
            for (const auto& bp : blitPasses)
            {
                unloadBlitPassRenderTargets(bp, sceneId);
            }

            StreamTextureHandleVector streamTextures;
            sceneResources.getAllStreamTextures(streamTextures);
            for (const auto& st : streamTextures)
            {
                unloadStreamTexture(st, sceneId);
            }

            DataBufferHandleVector dataBuffers;
            sceneResources.getAllDataBuffers(dataBuffers);
            for (const auto db : dataBuffers)
            {
                unloadDataBuffer(db, sceneId);
            }

            TextureBufferHandleVector textureBuffers;
            sceneResources.getAllTextureBuffers(textureBuffers);
            for (const auto tb : textureBuffers)
            {
                unloadTextureBuffer(tb, sceneId);
            }

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

    Bool RendererResourceManager::hasResourcesToBeUploaded() const
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
        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        IDevice& device = m_renderBackend.getDevice();
        const DeviceResourceHandle deviceHandle = device.uploadRenderBuffer(renderBuffer);

        UInt32 memSize = renderBuffer.width * renderBuffer.height * GetTexelSizeFromFormat(renderBuffer.format);
        const UInt32 sampleCount = renderBuffer.sampleCount;
        if (0 != sampleCount)
        {
            memSize *= sampleCount;
        }

        sceneResources.addRenderBuffer(renderBufferHandle, deviceHandle, memSize, ERenderBufferAccessMode_WriteOnly == renderBuffer.accessMode);
        m_stats.sceneResourceUploaded(sceneId, memSize);
    }

    void RendererResourceManager::unloadRenderTargetBuffer(RenderBufferHandle renderBufferHandle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        IDevice& device = m_renderBackend.getDevice();
        device.deleteRenderBuffer(sceneResources.getRenderBufferDeviceHandle(renderBufferHandle));
        sceneResources.removeRenderBuffer(renderBufferHandle);
    }

    void RendererResourceManager::uploadRenderTarget(RenderTargetHandle renderTarget, const RenderBufferHandleVector& rtBufferHandles, SceneId sceneId)
    {
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
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        const DeviceResourceHandle rtDeviceHandle = sceneResources.getRenderTargetDeviceHandle(renderTarget);
        IDevice& device = m_renderBackend.getDevice();
        device.deleteRenderTarget(rtDeviceHandle);
        sceneResources.removeRenderTarget(renderTarget);
    }

    void RendererResourceManager::uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType)
    {
        assert(!m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();

        m_offscreenBuffers.allocate(bufferHandle);
        OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);

        offscreenBufferDesc.m_colorBufferHandle[0] = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, sampleCount));
        switch (depthStencilBufferType)
        {
        case ERenderBufferType_InvalidBuffer:
            offscreenBufferDesc.m_depthBufferHandle = {};
            break;
        case ERenderBufferType_DepthBuffer:
            offscreenBufferDesc.m_depthBufferHandle = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_WriteOnly, sampleCount));
            break;
        case ERenderBufferType_DepthStencilBuffer:
            offscreenBufferDesc.m_depthBufferHandle = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, sampleCount));
            break;
        case ERenderBufferType_ColorBuffer:
        case ERenderBufferType_NUMBER_OF_ELEMENTS:
            assert(false);
        }

        const UInt32 sampleMultiplier = (sampleCount == 0) ? 1u : sampleCount;
        offscreenBufferDesc.m_estimatedVRAMUsage = width * height * ((isDoubleBuffered ? 2u : 1u) * GetTexelSizeFromFormat(ETextureFormat::RGBA8) * sampleMultiplier + GetTexelSizeFromFormat(ETextureFormat::Depth24_Stencil8) * sampleMultiplier);

        DeviceHandleVector bufferDeviceHandles;
        bufferDeviceHandles.push_back(offscreenBufferDesc.m_colorBufferHandle[0]);
        if(offscreenBufferDesc.m_depthBufferHandle.isValid())
            bufferDeviceHandles.push_back(offscreenBufferDesc.m_depthBufferHandle);
        offscreenBufferDesc.m_renderTargetHandle[0] = device.uploadRenderTarget(bufferDeviceHandles);

        if (isDoubleBuffered)
        {
            offscreenBufferDesc.m_colorBufferHandle[1] = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, sampleCount));

            DeviceHandleVector bufferDeviceHandles2;
            bufferDeviceHandles2.push_back(offscreenBufferDesc.m_colorBufferHandle[1]);
            if(offscreenBufferDesc.m_depthBufferHandle.isValid())
                bufferDeviceHandles2.push_back(offscreenBufferDesc.m_depthBufferHandle);
            offscreenBufferDesc.m_renderTargetHandle[1] = device.uploadRenderTarget(bufferDeviceHandles2);
        }

        // TODO Violin this code could be optimized if needed... No need to have two additional FBO activations just to set their clear state
        // Initial clear of the buffers
        device.activateRenderTarget(offscreenBufferDesc.m_renderTargetHandle[0]);
        device.colorMask(true, true, true, true);
        device.clearColor({ 0.f, 0.f, 0.f, 1.f });
        device.depthWrite(EDepthWrite::Enabled);
        device.scissorTest(EScissorTest::Disabled, {});
        device.clear(EClearFlags_All);

        if (isDoubleBuffered)
        {
            device.activateRenderTarget(offscreenBufferDesc.m_renderTargetHandle[1]);
            device.colorMask(true, true, true, true);
            device.clearColor({ 0.f, 0.f, 0.f, 1.f });
            device.depthWrite(EDepthWrite::Enabled);
            device.scissorTest(EScissorTest::Disabled, {});
            device.clear(EClearFlags_All);
            device.pairRenderTargetsForDoubleBuffering(offscreenBufferDesc.m_renderTargetHandle, offscreenBufferDesc.m_colorBufferHandle);
        }
    }

    void RendererResourceManager::unloadOffscreenBuffer(OffscreenBufferHandle bufferHandle)
    {
        assert(m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();
        const OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);

        device.deleteRenderTarget(offscreenBufferDesc.m_renderTargetHandle[0]);
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

        m_offscreenBuffers.release(bufferHandle);
    }

    void RendererResourceManager::uploadStreamBuffer(StreamBufferHandle bufferHandle, WaylandIviSurfaceId source)
    {
        assert(!m_streamBuffers.isAllocated(bufferHandle));
        m_streamBuffers.allocate(bufferHandle);
        *m_streamBuffers.getMemory(bufferHandle) = source;

        m_embeddedCompositingManager.refStream(source);

        assert(!contains_c(m_streamUsages[source].streamBufferUsages, bufferHandle));
        m_streamUsages[source].streamBufferUsages.push_back(bufferHandle);
    }

    void RendererResourceManager::unloadStreamBuffer(StreamBufferHandle bufferHandle)
    {
        assert(m_streamBuffers.isAllocated(bufferHandle));
        const auto source = *m_streamBuffers.getMemory(bufferHandle);
        m_streamBuffers.release(bufferHandle);

        m_embeddedCompositingManager.unrefStream(source);

        auto& streamUsage = m_streamUsages[source].streamBufferUsages;
        assert(contains_c(streamUsage, bufferHandle));
        streamUsage.erase(find_c(streamUsage, bufferHandle));
    }

    void RendererResourceManager::uploadStreamTexture(StreamTextureHandle handle, WaylandIviSurfaceId source, SceneId sceneId)
    {
        assert(handle.isValid());
        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        sceneResources.addStreamTexture(handle, source);

        m_embeddedCompositingManager.refStream(source);

        assert(!contains_c(m_streamUsages[source].sceneUsages[sceneId], handle));
        m_streamUsages[source].sceneUsages[sceneId].push_back(handle);
    }

    void RendererResourceManager::unloadStreamTexture(StreamTextureHandle handle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        const WaylandIviSurfaceId source = sceneResources.getStreamTextureSourceId(handle);
        sceneResources.removeStreamTexture(handle);

        m_embeddedCompositingManager.unrefStream(source);

        auto& streamUsage = m_streamUsages[source].sceneUsages[sceneId];
        assert(contains_c(streamUsage, handle));
        streamUsage.erase(find_c(streamUsage, handle));
        if (streamUsage.empty())
            m_streamUsages[source].sceneUsages.erase(sceneId);
    }

    void RendererResourceManager::uploadBlitPassRenderTargets(BlitPassHandle blitPass, RenderBufferHandle sourceRenderBuffer, RenderBufferHandle destinationRenderBuffer, SceneId sceneId)
    {
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

        sceneResources.addBlitPass(blitPass, blitRtSource, blitRtDest);
    }

    void RendererResourceManager::unloadBlitPassRenderTargets(BlitPassHandle blitPass, SceneId sceneId)
    {
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

    void RendererResourceManager::uploadDataBuffer(DataBufferHandle dataBufferHandle, EDataBufferType dataBufferType, EDataType dataType, UInt32 dataSizeInBytes, SceneId sceneId)
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

    void RendererResourceManager::updateDataBuffer(DataBufferHandle handle, UInt32 dataSizeInBytes, const Byte* data, SceneId sceneId)
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

    void RendererResourceManager::uploadTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 width, UInt32 height, ETextureFormat textureFormat, UInt32 mipLevelCount, SceneId sceneId)
    {
        assert(textureBufferHandle.isValid());
        assert(ETextureFormat::Invalid != textureFormat);
        assert(!IsFormatCompressed(textureFormat)); //can not be a compressed format

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        const UInt32 totalSizeInBytes = TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(textureFormat), width, height, 1u, mipLevelCount);

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle = device.allocateTexture2D(width, height, textureFormat, DefaultTextureSwizzleArray, mipLevelCount, totalSizeInBytes);
        assert(deviceHandle.isValid());

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

    void RendererResourceManager::updateTextureBuffer(TextureBufferHandle textureBufferHandle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getTextureBufferDeviceHandle(textureBufferHandle);
        assert(deviceHandle.isValid());

        IDevice& device = m_renderBackend.getDevice();
        device.bindTexture(deviceHandle);
        device.uploadTextureData(deviceHandle, mipLevel, x, y, 0u, width, height, 1u, data, 0u);

        const UInt32 updateDataSizeInBytes = TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(sceneResources.getTextureBufferFormat(textureBufferHandle)), width, height, 1u, 1u);
        m_stats.sceneResourceUploaded(sceneId, updateDataSizeInBytes);
    }

    DeviceResourceHandle RendererResourceManager::getTextureBufferDeviceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId) const
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        const RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        return sceneResources.getTextureBufferDeviceHandle(textureBufferHandle);
    }
}
