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
#include "RendererFramework/IResourceProvider.h"
#include "Utils/LogMacros.h"
#include "Utils/TextureMathUtils.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    RendererResourceManager::RendererResourceManager(
        IResourceProvider& resourceProvider,
        IResourceUploader& uploader,
        IRenderBackend& renderBackend,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        RequesterID requesterId,
        Bool keepEffects,
        const FrameTimer& frameTimer,
        RendererStatistics& stats,
        UInt64 clientResourceCacheSize)
        : m_id(requesterId)
        , m_resourceProvider(resourceProvider)
        , m_renderBackend(renderBackend)
        , m_embeddedCompositingManager(embeddedCompositingManager)
        , m_resourceUploadingManager(m_clientResourceRegistry, uploader, renderBackend, keepEffects, frameTimer, stats, clientResourceCacheSize)
        , m_stats(stats)
    {
    }

    RendererResourceManager::~RendererResourceManager()
    {
        assert(m_sceneResourceRegistryMap.count() == 0u);

        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::~RendererResourceManager Destroying offscreen buffers");
        for (OffscreenBufferHandle handle(0u); handle < m_offscreenBuffers.getTotalCount(); ++handle)
        {
            if (m_offscreenBuffers.isAllocated(handle))
                unloadOffscreenBuffer(handle);
        }

        for(const auto& resDesc : m_clientResourceRegistry.getAllResourceDescriptors())
        {
            UNUSED(resDesc);
            assert(resDesc.value.sceneUsage.empty());
        }
    }

    const RequesterID& RendererResourceManager::getRequesterID() const
    {
        return m_id;
    }

    void RendererResourceManager::referenceClientResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
    {
        for (const auto& resHash : resources)
        {
            if (!m_clientResourceRegistry.containsResource(resHash))
            {
                m_clientResourceRegistry.registerResource(resHash);
            }
            m_clientResourceRegistry.addResourceRef(resHash, sceneId);
        }
    }

    void RendererResourceManager::unreferenceClientResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
    {
        for (const auto& resHash : resources)
        {
            m_clientResourceRegistry.removeResourceRef(resHash, sceneId);
        }
    }

    void RendererResourceManager::groupResourcesBySceneId(const ResourceContentHashVector& resources, ResourcesPerSceneMap& resourcesPerScene) const
    {
        for(const auto& resHash : resources)
        {
            const ResourceDescriptor& resDesc = m_clientResourceRegistry.getResourceDescriptor(resHash);
            for(const auto& sceneUsage : resDesc.sceneUsage)
            {
                if (!resourcesPerScene.contains(sceneUsage))
                {
                    resourcesPerScene.put(sceneUsage, ResourceContentHashVector());
                }
                resourcesPerScene.get(sceneUsage)->push_back(resHash);
            }
        }
    }

    void RendererResourceManager::requestResourcesFromProvider(const ResourceContentHashVector& resources)
    {
        if (!resources.empty())
        {
            LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::requestAndUnrequestPendingResources Requesting " << resources.size() << " resources from resource provider");

            ResourcesPerSceneMap resourcesPerScene;
            groupResourcesBySceneId(resources, resourcesPerScene);

            for(const auto& res : resourcesPerScene)
            {
                m_resourceProvider.requestResourceAsyncronouslyFromFramework(res.value, m_id, res.key);
            }
        }
    }

    RendererSceneResourceRegistry& RendererResourceManager::getSceneResourceRegistry(SceneId sceneId)
    {
        if (!m_sceneResourceRegistryMap.contains(sceneId))
        {
            m_sceneResourceRegistryMap.put(sceneId, RendererSceneResourceRegistry());
        }
        return *m_sceneResourceRegistryMap.get(sceneId);
    }

    void RendererResourceManager::getRequestedResourcesAlreadyInCache(const IRendererResourceCache* cache)
    {
        if (!cache)
        {
            return;
        }

        const ResourceContentHashVector resourcesToBeRequested = m_clientResourceRegistry.getAllRegisteredResources();

        for (auto res : resourcesToBeRequested)
        {
            UInt32 resourceSize = 0;
            if (cache->hasResource(res, resourceSize))
            {
                ManagedResource newResource = RendererResourceManagerUtils::TryLoadResource(res, resourceSize, cache);

                if (newResource.getResourceObject() == nullptr)
                {
                    assert(false);
                    LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager::getRequestedResourcesAlreadyInCache. Failed to deserialize data from cache: #" << StringUtils::HexFromResourceContentHash(res));
                    return;
                }

                // Mimic the same state changes as if the resource had been requested and received over network
                m_clientResourceRegistry.setResourceStatus(res, EResourceStatus_Requested);
                m_clientResourceRegistry.setResourceData(res, newResource, DeviceResourceHandle::Invalid(), newResource.getResourceObject()->getTypeID());
                m_clientResourceRegistry.setResourceStatus(res, EResourceStatus_Provided);
            }
        }
    }

    void RendererResourceManager::requestAndUnrequestPendingClientResources()
    {
        // TODO vaclav/tobias find better solution - either resend or delay destruction of resources on client side that are pending on renderer
        // Workaround to avoid waiting for resources forever.
        // In case client destroys a resource before a request comes from renderer, it will report resource as unavailable
        // and renderer might get stuck in case of mapping state or sync flush waiting for this resource.
        // If the client creates the resource again there is currently no mechanism to let the renderer know or send the resource
        // to the renderer waiting for it.
        // Therefore pending resources are re-requested when they haven't arrived for N frames
        ++m_frameCounter;

        // collect resources to re-request
        ResourceContentHashVector resourcesToBeRequested;
        resourcesToBeRequested.reserve(m_clientResourceRegistry.getAllRequestedResources().size());
        for (const auto& hash : m_clientResourceRegistry.getAllRequestedResources())
        {
            const UInt64 frameOfLastRequest = m_clientResourceRegistry.getResourceDescriptor(hash).lastRequestFrameIdx;
            if (frameOfLastRequest + m_numberOfFramesToRerequestResource <= m_frameCounter)
            {
                resourcesToBeRequested.push_back(hash);
            }
        }

        // add newly registered resources
        const ResourceContentHashVector& registeredResources = m_clientResourceRegistry.getAllRegisteredResources();
        resourcesToBeRequested.insert(resourcesToBeRequested.end(), registeredResources.begin(), registeredResources.end());
        if (!resourcesToBeRequested.empty())
        {
            requestResourcesFromProvider(resourcesToBeRequested);

            for (const auto& hash : resourcesToBeRequested)
            {
                m_clientResourceRegistry.setResourceStatus(hash, EResourceStatus_Requested, m_frameCounter);
            }
        }

        const ResourceContentHashVector& unusedResources = m_clientResourceRegistry.getAllResourcesNotInUseByScenesAndNotUploaded();
        while(!unusedResources.empty())
        {
            const ResourceContentHash hash = unusedResources.front();
            const ResourceDescriptor& rd = m_clientResourceRegistry.getResourceDescriptor(hash);

            if (rd.status == EResourceStatus_Requested)
            {
                LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::requestAndUnrequestPendingResources Canceling request for resource" << StringUtils::HexFromResourceContentHash(hash));
                m_resourceProvider.cancelResourceRequest(hash, m_id);
            }

            assert(rd.status != EResourceStatus_Uploaded);
            LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::requestAndUnrequestPendingResources Removing resource descriptor for resource #" << StringUtils::HexFromResourceContentHash(hash));
            m_clientResourceRegistry.unregisterResource(hash);
        }
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

    void RendererResourceManager::unreferenceAllClientResourcesForScene(SceneId sceneId)
    {
        for (const auto& resDesc : m_clientResourceRegistry.getAllResourceDescriptors())
        {
            if (contains_c(resDesc.value.sceneUsage, sceneId))
            {
                m_clientResourceRegistry.removeResourceRef(resDesc.key, sceneId);
            }
        }
    }

    void RendererResourceManager::processArrivedClientResources(IRendererResourceCache* cache)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::checkForArrivedResources Checking for arrived resources");

        //take arrived resources
        const ManagedResourceVector arrivedResources = m_resourceProvider.popArrivedResources(m_id);
        for (const ManagedResource& current : arrivedResources)
        {
            const IResource* resourceObject = current.getResourceObject();
            const ResourceContentHash resHash = resourceObject->getHash();

            if (m_clientResourceRegistry.containsResource(resHash))
            {
                if (m_clientResourceRegistry.getResourceStatus(resHash) == EResourceStatus_Requested)
                {
                    m_clientResourceRegistry.setResourceData(resHash, current, DeviceResourceHandle::Invalid(), resourceObject->getTypeID());
                    m_clientResourceRegistry.setResourceStatus(resHash, EResourceStatus_Provided);

                    // Update local resource cache with the received resource
                    if (cache)
                    {
                        // The resource should be requested by one or more scenes
                        if (!m_clientResourceRegistry.getResourceDescriptor(resHash).sceneUsage.empty())
                        {
                            // There might be multiple scenes using the same resource. We have no way
                            // of knowing which one the user meant, so we just pick the first one.
                            const SceneId sceneId(m_clientResourceRegistry.getResourceDescriptor(resHash).sceneUsage.front());
                            RendererResourceManagerUtils::StoreResource(cache, resourceObject, sceneId);
                        }
                    }
                }
                else
                {
                    // update status information
                    ++m_numberOfArrivedResourcesInWrongStatus;
                    UInt32 resSize = resourceObject->isCompressedAvailable() ? resourceObject->getCompressedDataSize() : resourceObject->getDecompressedDataSize();
                    m_sizeOfArrivedResourcesInWrongStatus += resSize;

                    // This might indicate that the resource status is messed up - indicates a logic error
                    LOG_WARN(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::checkForArrivedResources Wrong status of arrived resource #" << StringUtils::HexFromResourceContentHash(resHash) <<
                             "; Status: " << EnumToString(m_clientResourceRegistry.getResourceStatus(resHash)) << "; resourceSize: " << resSize <<
                             "; numberWrongStatus: " << m_numberOfArrivedResourcesInWrongStatus << "; sumSizeWrongStatus: " << m_sizeOfArrivedResourcesInWrongStatus);
                }
            }
            else
            {
                // This can be useful for resource prefetching when client sends them ahead of scene.
                // Slight conceptual change would be required that would allow to manage a resource
                // without being used by any scene. If the resource is never actually used, it will become
                // a zombie that is only released at destruction time.

                // This indicates error - resource either arrived before the scene itself (or was prefetches)
                // OR resource was unrequested, but still arrived from network
                LOG_ERROR(CONTEXT_RENDERER, "RendererResourceManager[" << m_id << "]::checkForArrivedResources Descriptor for arrived resource " << StringUtils::HexFromResourceContentHash(resHash) << " does not exist");
            }
        }
    }

    Bool RendererResourceManager::hasClientResourcesToBeUploaded() const
    {
        return m_resourceUploadingManager.hasAnythingToUpload();
    }

    void RendererResourceManager::uploadAndUnloadPendingClientResources()
    {
        m_resourceUploadingManager.uploadAndUnloadPendingResources();
    }

    EResourceStatus RendererResourceManager::getClientResourceStatus(const ResourceContentHash& hash) const
    {
        return m_clientResourceRegistry.getResourceStatus(hash);
    }

    EResourceType RendererResourceManager::getClientResourceType(const ResourceContentHash& hash) const
    {
        return m_clientResourceRegistry.getResourceDescriptor(hash).type;
    }

    DeviceResourceHandle RendererResourceManager::getClientResourceDeviceHandle(const ResourceContentHash& hash) const
    {
        return m_clientResourceRegistry.getResourceDescriptor(hash).deviceHandle;
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

    void RendererResourceManager::uploadOffscreenBuffer(OffscreenBufferHandle bufferHandle, UInt32 width, UInt32 height, Bool isDoubleBuffered)
    {
        assert(!m_offscreenBuffers.isAllocated(bufferHandle));
        IDevice& device = m_renderBackend.getDevice();

        m_offscreenBuffers.allocate(bufferHandle);
        OffscreenBufferDescriptor& offscreenBufferDesc = *m_offscreenBuffers.getMemory(bufferHandle);

        offscreenBufferDesc.m_colorBufferHandle[0] = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
        offscreenBufferDesc.m_depthBufferHandle = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_DepthStencilBuffer, ETextureFormat_Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u));
        offscreenBufferDesc.m_estimatedVRAMUsage = width * height * ((isDoubleBuffered ? 2u : 1u) * GetTexelSizeFromFormat(ETextureFormat_RGBA8) + GetTexelSizeFromFormat(ETextureFormat_Depth24_Stencil8));

        DeviceHandleVector bufferDeviceHandles;
        bufferDeviceHandles.push_back(offscreenBufferDesc.m_colorBufferHandle[0]);
        bufferDeviceHandles.push_back(offscreenBufferDesc.m_depthBufferHandle);
        offscreenBufferDesc.m_renderTargetHandle[0] = device.uploadRenderTarget(bufferDeviceHandles);

        if (isDoubleBuffered)
        {
            offscreenBufferDesc.m_colorBufferHandle[1] = device.uploadRenderBuffer(RenderBuffer(width, height, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));

            DeviceHandleVector bufferDeviceHandles2;
            bufferDeviceHandles2.push_back(offscreenBufferDesc.m_colorBufferHandle[1]);
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
        device.deleteRenderBuffer(offscreenBufferDesc.m_depthBufferHandle);
        if (offscreenBufferDesc.m_colorBufferHandle[1].isValid())
        {
            device.deleteRenderBuffer(offscreenBufferDesc.m_colorBufferHandle[1]);
        }

        m_offscreenBuffers.release(bufferHandle);
    }

    void RendererResourceManager::uploadStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source, SceneId sceneId)
    {
        assert(handle.isValid());
        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);
        sceneResources.addStreamTexture(handle, source);

        m_embeddedCompositingManager.uploadStreamTexture(handle, source, sceneId);
    }

    void RendererResourceManager::unloadStreamTexture(StreamTextureHandle handle, SceneId sceneId)
    {
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);
        const StreamTextureSourceId source = sceneResources.getStreamTextureSourceId(handle);
        sceneResources.removeStreamTexture(handle);

        m_embeddedCompositingManager.deleteStreamTexture(handle, source, sceneId);
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
        assert(EDataType_Invalid != dataType);

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle;
        switch (dataBufferType)
        {
        case EDataBufferType::IndexBuffer:
            deviceHandle = device.allocateIndexBuffer(dataType, dataSizeInBytes);
            break;
        case EDataBufferType::VertexBuffer:
            deviceHandle = device.allocateVertexBuffer(dataType, dataSizeInBytes);
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
        assert(ETextureFormat_Invalid != textureFormat);
        assert(!IsFormatCompressed(textureFormat)); //can not be a compressed format

        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        const UInt32 totalSizeInBytes = TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(textureFormat), width, height, 1u, mipLevelCount);

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle = device.allocateTexture2D(width, height, textureFormat, mipLevelCount, totalSizeInBytes);
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

    void RendererResourceManager::uploadTextureSampler(TextureSamplerHandle handle, SceneId sceneId, const TextureSamplerStates& states)
    {
        assert(handle.isValid());
        RendererSceneResourceRegistry& sceneResources = getSceneResourceRegistry(sceneId);

        const EWrapMethod     wrapU           = states.m_addressModeU;
        const EWrapMethod     wrapV           = states.m_addressModeV;
        const EWrapMethod     wrapR           = states.m_addressModeR;
        const ESamplingMethod minSampling     = states.m_minSamplingMode;
        const ESamplingMethod magSampling     = states.m_magSamplingMode;
        const UInt32          anisotropyLevel = states.m_anisotropyLevel;

        IDevice& device = m_renderBackend.getDevice();
        DeviceResourceHandle deviceHandle = device.uploadTextureSampler(wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel);
        assert(deviceHandle.isValid());

        sceneResources.addTextureSampler(handle, deviceHandle);
    }

    void RendererResourceManager::unloadTextureSampler(TextureSamplerHandle handle, SceneId sceneId)
    {
        assert(handle.isValid());
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        const DeviceResourceHandle deviceHandle = sceneResources.getTextureSamplerDeviceHandle(handle);
        assert(deviceHandle.isValid());

        IDevice& device = m_renderBackend.getDevice();
        device.deleteTextureSampler(deviceHandle);
        sceneResources.removeTextureSampler(handle);
    }

    DeviceResourceHandle RendererResourceManager::getTextureSamplerDeviceHandle(TextureSamplerHandle handle, SceneId sceneId) const
    {
        assert(handle.isValid());
        assert(m_sceneResourceRegistryMap.contains(sceneId));
        RendererSceneResourceRegistry& sceneResources = *m_sceneResourceRegistryMap.get(sceneId);

        return sceneResources.getTextureSamplerDeviceHandle(handle);
    }
}
