//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ResourceCachedScene.h"
#include "RendererLib/IResourceDeviceHandleAccessor.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "Common/Cpp11Macros.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ResourceCachedScene::ResourceCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : DataReferenceLinkCachedScene(sceneLinksManager, sceneInfo)
        , m_renderableResourcesDirtinessNeedsUpdate(false)
        , m_renderTargetsDirty(false)
        , m_blitPassesDirty(false)
    {
    }

    template <typename T>
    void resizeContainerIfSmaller(T& container, UInt32 newSize)
    {
        if (newSize > container.size())
        {
            container.resize(newSize);
        }
    }

    void ResourceCachedScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        DataReferenceLinkCachedScene::preallocateSceneSize(sizeInfo);

        resizeContainerIfSmaller(m_renderableResourcesDirty, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_dataInstancesDirty, sizeInfo.datainstanceCount);
        resizeContainerIfSmaller(m_textureSamplersDirty, sizeInfo.textureSamplerCount);
        resizeContainerIfSmaller(m_effectDeviceHandleCache, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_deviceHandleCacheForVertexAttributes, sizeInfo.datainstanceCount);
        resizeContainerIfSmaller(m_deviceHandleCacheForTextures, sizeInfo.textureSamplerCount);
        resizeContainerIfSmaller(m_renderTargetCache, sizeInfo.renderTargetCount);
        resizeContainerIfSmaller(m_blitPassCache, sizeInfo.blitPassCount * 2u);
    }

    RenderableHandle ResourceCachedScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle /*= RenderableHandle::Invalid()*/)
    {
        const RenderableHandle renderable = DataReferenceLinkCachedScene::allocateRenderable(nodeHandle, handle);

        const UInt32 indexIntoCache = renderable.asMemoryHandle();
        assert(indexIntoCache < m_effectDeviceHandleCache.size());
        m_effectDeviceHandleCache[indexIntoCache] = DeviceResourceHandle::Invalid();
        setRenderableResourcesDirtyFlag(renderable, true);
        return renderable;
    }

    void ResourceCachedScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        DataReferenceLinkCachedScene::releaseRenderable(renderableHandle);
        setRenderableResourcesDirtyFlag(renderableHandle, false);
    }

    DataInstanceHandle ResourceCachedScene::allocateDataInstance(DataLayoutHandle handle, DataInstanceHandle instanceHandle /*= DataInstanceHandle::Invalid()*/)
    {
        const DataInstanceHandle dataInstance = DataReferenceLinkCachedScene::allocateDataInstance(handle, instanceHandle);
        const DataLayout& layout = getDataLayout(handle);
        const UInt32 fieldCount = layout.getFieldCount();
        if (fieldCount > 0u && isGeometryDataLayout(layout))
        {
            const UInt32 indexIntoCache = dataInstance.asMemoryHandle();
            assert(indexIntoCache < m_deviceHandleCacheForVertexAttributes.size());
            m_deviceHandleCacheForVertexAttributes[indexIntoCache].resize(fieldCount);
            for (UInt32 i = 0; i < fieldCount; ++i)
            {
                m_deviceHandleCacheForVertexAttributes[indexIntoCache][i] = DeviceResourceHandle::Invalid();
            }
        }

        setDataInstanceDirtyFlag(dataInstance, true);

        return dataInstance;
    }

    void ResourceCachedScene::releaseDataInstance(DataInstanceHandle dataInstanceHandle)
    {
        DataReferenceLinkCachedScene::releaseDataInstance(dataInstanceHandle);
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    TextureSamplerHandle ResourceCachedScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        const TextureSamplerHandle actualHandle = DataReferenceLinkCachedScene::allocateTextureSampler(sampler, handle);

        const UInt32 indexIntoCache = actualHandle.asMemoryHandle();
        assert(indexIntoCache < m_deviceHandleCacheForTextures.size());
        m_deviceHandleCacheForTextures[indexIntoCache] = DeviceResourceHandle::Invalid();
        setTextureSamplerDirtyFlag(actualHandle, true);

        return actualHandle;
    }

    void ResourceCachedScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        setTextureSamplerDirtyFlag(handle, true);
        DataReferenceLinkCachedScene::releaseTextureSampler(handle);
    }

    void ResourceCachedScene::releaseStreamTexture(StreamTextureHandle handle)
    {
        setRenderableResourcesDirtyByStreamTexture(handle);
        DataReferenceLinkCachedScene::releaseStreamTexture(handle);
    }

    void ResourceCachedScene::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        DataReferenceLinkCachedScene::setRenderableEffect(renderableHandle, effectHash);

        const UInt32 indexIntoCache = renderableHandle.asMemoryHandle();
        assert(indexIntoCache < m_effectDeviceHandleCache.size());
        m_effectDeviceHandleCache[indexIntoCache] = DeviceResourceHandle::Invalid();

        setRenderableResourcesDirtyFlag(renderableHandle, true);
    }

    void ResourceCachedScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        DataReferenceLinkCachedScene::setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        setRenderableResourcesDirtyFlag(renderableHandle, true);
    }

    void ResourceCachedScene::setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        DataReferenceLinkCachedScene::setDataResource(dataInstanceHandle, field, hash, dataBuffer, instancingDivisor);

        const UInt32 indexIntoCache = dataInstanceHandle.asMemoryHandle();
        assert(indexIntoCache < m_deviceHandleCacheForVertexAttributes.size());

        //TODO violin this is too implicit
        DeviceHandleVector& instanceDeviceCache = m_deviceHandleCacheForVertexAttributes[indexIntoCache];
        const UInt32 indexIntoBufferCache = field.asMemoryHandle();
        assert(indexIntoBufferCache < instanceDeviceCache.size());
        instanceDeviceCache[indexIntoBufferCache] = DeviceResourceHandle::Invalid();
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    void ResourceCachedScene::setDataTextureSamplerHandle(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        DataReferenceLinkCachedScene::setDataTextureSamplerHandle(dataInstanceHandle, field, samplerHandle);
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    void ResourceCachedScene::setForceFallbackImage(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "ResourceCachedScene::setForceFallbackImage(): setting force fallback to :" << forceFallbackImage << " for stream texture :" << streamTextureHandle.asMemoryHandle()
                  << " with source id :" << getStreamTexture(streamTextureHandle).source);
        DataReferenceLinkCachedScene::setForceFallbackImage(streamTextureHandle, forceFallbackImage);
        setRenderableResourcesDirtyByStreamTexture(streamTextureHandle);
    }

    RenderTargetHandle ResourceCachedScene::allocateRenderTarget(RenderTargetHandle targetHandle)
    {
        const RenderTargetHandle rtHandle = DataReferenceLinkCachedScene::allocateRenderTarget(targetHandle);

        const UInt32 indexIntoCache = rtHandle.asMemoryHandle();
        assert(indexIntoCache < m_renderTargetCache.size());
        m_renderTargetCache[indexIntoCache] = DeviceResourceHandle::Invalid();
        m_renderTargetsDirty = true;

        return rtHandle;
    }

    BlitPassHandle ResourceCachedScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle blitPassHandle = DataReferenceLinkCachedScene::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);

        const UInt32 indexIntoCache = blitPassHandle.asMemoryHandle() * 2u;
        assert(indexIntoCache + 1u < m_blitPassCache.size());
        m_blitPassCache[indexIntoCache]      = DeviceResourceHandle::Invalid();
        m_blitPassCache[indexIntoCache + 1u] = DeviceResourceHandle::Invalid();
        m_blitPassesDirty = true;

        return blitPassHandle;
    }

    Bool ResourceCachedScene::renderableResourcesDirty(RenderableHandle handle) const
    {
        UInt32 renderableAsIndex = handle.asMemoryHandle();
        return m_renderableResourcesDirty[renderableAsIndex];
    }

    Bool ResourceCachedScene::renderableResourcesDirty(const RenderableVector& handles) const
    {
        for (const auto handle : handles)
        {
            if (renderableResourcesDirty(handle))
                return true;
        }

        return false;
    }

    DeviceResourceHandle ResourceCachedScene::getRenderableEffectDeviceHandle(RenderableHandle renderable) const
    {
        const UInt32 renderableAsIndex = renderable.asMemoryHandle();
        assert(renderableAsIndex < m_effectDeviceHandleCache.size());
        return m_effectDeviceHandleCache[renderableAsIndex];
    }

    const DeviceHandleCache& ResourceCachedScene::getCachedHandlesForVertexAttributes() const
    {
        return m_deviceHandleCacheForVertexAttributes;
    }

    const DeviceHandleVector& ResourceCachedScene::getCachedHandlesForTextureSamplers() const
    {
        return m_deviceHandleCacheForTextures;
    }

    const DeviceHandleVector& ResourceCachedScene::getCachedHandlesForRenderTargets() const
    {
        return m_renderTargetCache;
    }

    const DeviceHandleVector& ResourceCachedScene::getCachedHandlesForBlitPassRenderTargets() const
    {
        return m_blitPassCache;
    }

    Bool ResourceCachedScene::CheckAndUpdateDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash)
    {
        if (!deviceHandleInOut.isValid())
        {
            if (!resourceHash.isValid())
            {
                return false;
            }

            const DeviceResourceHandle deviceHandle = resourceAccessor.getClientResourceDeviceHandle(resourceHash);

            if (!deviceHandle.isValid())
            {
                return false;
            }

            deviceHandleInOut = deviceHandle;
        }

        return true;
    }

    Bool ResourceCachedScene::CheckAndUpdateBufferDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash, SceneId sceneId, DataBufferHandle dataBufferHandle)
    {
        if (deviceHandleInOut.isValid())
        {
            return true;
        }

        if (resourceHash.isValid())
        {
            deviceHandleInOut = resourceAccessor.getClientResourceDeviceHandle(resourceHash);
        }
        else if (dataBufferHandle.isValid())
        {
            deviceHandleInOut = resourceAccessor.getDataBufferDeviceHandle(dataBufferHandle, sceneId);
        }

        return deviceHandleInOut.isValid();
    }

    Bool ResourceCachedScene::checkAndUpdateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable)
    {
        const ResourceContentHash& hash = getRenderable(renderable).effectResource;
        return CheckAndUpdateDeviceHandle(resourceAccessor, m_effectDeviceHandleCache[renderable.asMemoryHandle()], hash);
    }

    Bool ResourceCachedScene::checkAndUpdateTextureResources(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, RenderableHandle renderable)
    {
        const DataInstanceHandle dataInstance = getRenderable(renderable).dataInstances[ERenderableDataSlotType_Uniforms];
        if (!dataInstance.isValid())
        {
            return false;
        }

        const DataLayoutHandle dataLayoutHandle = getLayoutOfDataInstance(dataInstance);
        const DataLayout& layout = getDataLayout(dataLayoutHandle);
        const UInt32 totalVertexArrayCount = layout.getFieldCount();
        for (DataFieldHandle dataField(0u); dataField < totalVertexArrayCount; ++dataField)
        {
            if (EDataType_TextureSampler == layout.getField(dataField).dataType)
            {
                const TextureSamplerHandle sampler = getDataTextureSamplerHandle(dataInstance, dataField);
                if (!sampler.isValid() || !isTextureSamplerAllocated(sampler) ||
                    !updateTextureSamplerResource(resourceAccessor, embeddedCompositingManager, sampler))
                {
                    return false;
                }
            }
        }

        return true;
    }

    Bool ResourceCachedScene::checkAndUpdateGeometryResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable)
    {
        const DataInstanceHandle dataInstance = getRenderable(renderable).dataInstances[ERenderableDataSlotType_Geometry];
        if (!dataInstance.isValid())
        {
            return false;
        }

        const DataLayoutHandle geometryLayoutHandle = getLayoutOfDataInstance(dataInstance);
        assert(geometryLayoutHandle.isValid());
        const DataLayout& geometryLayout = getDataLayout(geometryLayoutHandle);

        DeviceHandleVector& vertexAttributesCache = m_deviceHandleCacheForVertexAttributes[dataInstance.asMemoryHandle()];

        // there has to be always at least indices field in geometry data layout
        static const DataFieldHandle indicesDataField(0u);
        assert(EFixedSemantics_Indices == geometryLayout.getField(indicesDataField).semantics);
        assert(EDataType_Indices == geometryLayout.getField(indicesDataField).dataType);

        const UInt32 numberOfGeometryFields = geometryLayout.getFieldCount();
        assert(numberOfGeometryFields >= 1u);
        assert(numberOfGeometryFields == vertexAttributesCache.size());

        const SceneId sceneId = getSceneId();
        for (DataFieldHandle attributeField = indicesDataField; attributeField < numberOfGeometryFields; ++attributeField)
        {
            assert(IsBufferDataType(geometryLayout.getField(attributeField).dataType));
            const ResourceField& dataResource = getDataResource(dataInstance, attributeField);

            const bool isIndicesField = indicesDataField == attributeField;
            const bool usesIndices = dataResource.hash.isValid() || DataBufferHandle::Invalid() != dataResource.dataBuffer;

            if (isIndicesField && !usesIndices)
            {
                // Indices field but not using indices -> do not check availability of resource data for this field
                continue;
            }

            const bool deviceHandleValid = CheckAndUpdateBufferDeviceHandle(resourceAccessor, vertexAttributesCache[attributeField.asMemoryHandle()], dataResource.hash, sceneId, dataResource.dataBuffer);
            if (!deviceHandleValid)
            {
                return false;
            }
        }

        return true;
    }

    void ResourceCachedScene::checkAndUpdateRenderTargetResources(const IResourceDeviceHandleAccessor& resourceAccessor)
    {
        if (!m_renderTargetsDirty)
        {
            return;
        }

        const SceneId sceneId = getSceneId();

        const UInt32 rtCount = static_cast<UInt32>(m_renderTargetCache.size());
        for (RenderTargetHandle rtHandle(0u); rtHandle < rtCount; ++rtHandle)
        {
            DeviceResourceHandle& deviceHandle = m_renderTargetCache[rtHandle.asMemoryHandle()];
            if (isRenderTargetAllocated(rtHandle) && !deviceHandle.isValid())
            {
                deviceHandle = resourceAccessor.getRenderTargetDeviceHandle(rtHandle, sceneId);
                assert(deviceHandle.isValid());
            }
        }

        m_renderTargetsDirty = false;
    }

    void ResourceCachedScene::checkAndUpdateBlitPassResources(const IResourceDeviceHandleAccessor& resourceAccessor)
    {
        if (!m_blitPassesDirty)
        {
            return;
        }

        const SceneId sceneId = getSceneId();

        const UInt32 blitPassCount = getBlitPassCount();
        for (BlitPassHandle handle(0u); handle < blitPassCount; ++handle)
        {
            const UInt32 indexIntoCache = handle.asMemoryHandle() * 2u;
            DeviceResourceHandle& deviceHandleSrc = m_blitPassCache[indexIntoCache];
            DeviceResourceHandle& deviceHandleDst = m_blitPassCache[indexIntoCache + 1u];
            if (isBlitPassAllocated(handle) && (!deviceHandleSrc.isValid() || !deviceHandleDst.isValid()))
            {
                resourceAccessor.getBlitPassRenderTargetsDeviceHandle(handle, sceneId, deviceHandleSrc, deviceHandleDst);
                assert(deviceHandleSrc.isValid());
                assert(deviceHandleDst.isValid());
            }
        }

        m_blitPassesDirty = false;
    }

    Bool ResourceCachedScene::updateTextureSamplerResource(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, TextureSamplerHandle sampler)
    {
        assert(sampler.asMemoryHandle() < m_deviceHandleCacheForTextures.size());
        const TextureSampler& samplerData = getTextureSampler(sampler);

        switch (samplerData.contentType)
        {
        case TextureSampler::ContentType::ClientTexture:
            return resolveTextureSamplerResourceDeviceHandle(resourceAccessor, sampler, m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::TextureBuffer:
            return updateTextureSamplerResourceAsTextureBuffer(resourceAccessor, TextureBufferHandle(samplerData.contentHandle), m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::RenderBuffer:
            return updateTextureSamplerResourceAsRenderBuffer(resourceAccessor, RenderBufferHandle(samplerData.contentHandle), m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::StreamTexture:
            return updateTextureSamplerResourceAsStreamTexture(resourceAccessor, embeddedCompositingManager, StreamTextureHandle(samplerData.contentHandle), m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::OffscreenBuffer:
            m_deviceHandleCacheForTextures[sampler.asMemoryHandle()] = resourceAccessor.getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle(samplerData.contentHandle));
            return true;
        default:
            assert(false);
            return false;
        }
    }

    Bool ResourceCachedScene::updateTextureSamplerResourceAsRenderBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut)
    {
        assert(getRenderBuffer(bufferHandle).type != ERenderBufferType_InvalidBuffer);
        const DeviceResourceHandle textureDeviceHandle = resourceAccessor.getRenderTargetBufferDeviceHandle(bufferHandle, getSceneId());

        deviceHandleOut = textureDeviceHandle;
        return textureDeviceHandle.isValid();
    }

    Bool ResourceCachedScene::updateTextureSamplerResourceAsTextureBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const TextureBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut)
    {
        const DeviceResourceHandle textureDeviceHandle = resourceAccessor.getTextureBufferDeviceHandle(bufferHandle, getSceneId());

        deviceHandleOut = textureDeviceHandle;
        return textureDeviceHandle.isValid();
    }

    Bool ResourceCachedScene::updateTextureSamplerResourceAsStreamTexture(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager, const StreamTextureHandle streamTextureHandle, DeviceResourceHandle& deviceHandleInOut)
    {
        assert(isStreamTextureAllocated(streamTextureHandle));
        const StreamTexture& streamTexture = getStreamTexture(streamTextureHandle);
        const StreamTextureSourceId source(streamTexture.source);
        const DeviceResourceHandle streamTextureDeviceHandle = embeddedCompositingManager.getCompositedTextureDeviceHandleForStreamTexture(source);
        if (streamTexture.forceFallbackTexture)
        {
            LOG_INFO(CONTEXT_RENDERER, "ResourceCachedScene::updateTextureSamplerResourceAsStreamTexture(): using fallback texture for stream texture :" << streamTextureHandle.asMemoryHandle()
                      << " with source id :" << source.getValue() << " because force fallback is set");
            return CheckAndUpdateDeviceHandle(resourceAccessor, deviceHandleInOut, streamTexture.fallbackTexture);
        }
        else if (!streamTextureDeviceHandle.isValid())
        {
            LOG_INFO(CONTEXT_RENDERER, "ResourceCachedScene::updateTextureSamplerResourceAsStreamTexture(): using fallback texture for stream texture :" << streamTextureHandle.asMemoryHandle()
                      << " with source id :" << source.getValue() << " because stream source not available");
            return CheckAndUpdateDeviceHandle(resourceAccessor, deviceHandleInOut, streamTexture.fallbackTexture);
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "ResourceCachedScene::updateTextureSamplerResourceAsStreamTexture(): using composited texture for stream texture :" << streamTextureHandle.asMemoryHandle()
                      << " with source id :" << source.getValue());
        }


        deviceHandleInOut = streamTextureDeviceHandle;

        return true;
    }

    Bool ResourceCachedScene::resolveTextureSamplerResourceDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler, DeviceResourceHandle& deviceHandleInOut)
    {
        const ResourceContentHash& hash = getTextureSampler(sampler).textureResource;
        return CheckAndUpdateDeviceHandle(resourceAccessor, deviceHandleInOut, hash);
    }

    void ResourceCachedScene::updateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager)
    {
        updateRenderablesResourcesDirtiness();

        const UInt32 renderableCount = getRenderableCount();
        for (RenderableHandle renderable(0); renderable < renderableCount; ++renderable)
        {
            const UInt32 renderableAsIndex = renderable.asMemoryHandle();
            if (isRenderableAllocated(renderable) && m_renderableResourcesDirty[renderableAsIndex])
            {
                if (checkAndUpdateRenderableResources(resourceAccessor, renderable) &&
                    checkAndUpdateTextureResources(resourceAccessor, embeddedCompositingManager, renderable) &&
                    checkAndUpdateGeometryResources(resourceAccessor, renderable))
                {
                    setRenderableResourcesDirtyFlag(renderable, false);
                }
            }
        }

        checkAndUpdateRenderTargetResources(resourceAccessor);
        checkAndUpdateBlitPassResources(resourceAccessor);
    }

    void ResourceCachedScene::updateRenderablesResourcesDirtiness()
    {
        if (m_renderableResourcesDirtinessNeedsUpdate)
        {
            const UInt32 totalDataInstanceCount = getDataInstanceCount();
            for (DataInstanceHandle d(0u); d < totalDataInstanceCount; ++d)
            {
                if (!isDataInstanceDirty(d) && doesDataInstanceReferToDirtyTextureSampler(d))
                {
                    setDataInstanceDirtyFlag(d, true);
                }
            }

            const UInt32 totalRenderableCount = getRenderableCount();
            for (RenderableHandle r(0u); r < totalRenderableCount; ++r)
            {
                if (doesRenderableReferToDirtyDataInstance(r))
                {
                    setRenderableResourcesDirtyFlag(r, true);
                }
            }

            for (DataInstanceHandle d(0u); d < totalDataInstanceCount; ++d)
            {
                setDataInstanceDirtyFlag(d, false);
            }

            const UInt32 totalTextureSamplerCount = getTextureSamplerCount();
            for (TextureSamplerHandle t(0u); t < totalTextureSamplerCount; ++t)
            {
                setTextureSamplerDirtyFlag(t, false);
            }

            m_renderableResourcesDirtinessNeedsUpdate = false;
        }
    }

    void ResourceCachedScene::setRenderableResourcesDirtyFlag(RenderableHandle handle, Bool dirty) const
    {
        const UInt32 indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_renderableResourcesDirty.size());
        m_renderableResourcesDirty[indexIntoCache] = dirty;
    }

    void ResourceCachedScene::setDataInstanceDirtyFlag(DataInstanceHandle handle, Bool dirty) const
    {
        const UInt32 indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_dataInstancesDirty.size());
        m_dataInstancesDirty[indexIntoCache] = dirty;

        m_renderableResourcesDirtinessNeedsUpdate |= dirty;
    }

    void ResourceCachedScene::setTextureSamplerDirtyFlag(TextureSamplerHandle handle, Bool dirty) const
    {
        const UInt32 indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_textureSamplersDirty.size());
        m_textureSamplersDirty[indexIntoCache] = dirty;

        m_renderableResourcesDirtinessNeedsUpdate |= dirty;
    }

    Bool ResourceCachedScene::doesRenderableReferToDirtyDataInstance(RenderableHandle handle) const
    {
        if (isRenderableAllocated(handle))
        {
            const DataInstanceHandle uniformsDataInstance = getRenderable(handle).dataInstances[ERenderableDataSlotType_Uniforms];
            if (uniformsDataInstance.isValid() && isDataInstanceDirty(uniformsDataInstance))
            {
                return true;
            }

            const DataInstanceHandle geometryDataInstance = getRenderable(handle).dataInstances[ERenderableDataSlotType_Geometry];
            if (geometryDataInstance.isValid() && isDataInstanceDirty(geometryDataInstance))
            {
                return true;
            }
        }

        return false;
    }

    Bool ResourceCachedScene::isDataInstanceDirty(DataInstanceHandle handle) const
    {
        const UInt32 indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_dataInstancesDirty.size());
        return m_dataInstancesDirty[indexIntoCache];
    }

    Bool ResourceCachedScene::isTextureSamplerDirty(TextureSamplerHandle handle) const
    {
        const UInt32 indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_textureSamplersDirty.size());
        return m_textureSamplersDirty[indexIntoCache];
    }

    Bool ResourceCachedScene::isGeometryDataLayout(const DataLayout& layout) const
    {
        // TODO vaclav mark data layout explicitly if used for geometry or uniforms when network protocol can change
        // For now we check if indices field is contained to determine geometry data instance - this field exists even if no indices are actually used
        assert(layout.getFieldCount() > 0u);
        return layout.getField(DataFieldHandle(0u)).dataType == EDataType_Indices;
    }

    void ResourceCachedScene::setRenderableResourcesDirtyByTextureSampler(TextureSamplerHandle textureSamplerHandle) const
    {
        setTextureSamplerDirtyFlag(textureSamplerHandle, true);

        const UInt32 indexIntoCache = textureSamplerHandle.asMemoryHandle();
        assert(indexIntoCache < m_deviceHandleCacheForTextures.size());
        m_deviceHandleCacheForTextures[indexIntoCache] = DeviceResourceHandle::Invalid();
    }

    void ResourceCachedScene::setRenderableResourcesDirtyByStreamTexture(StreamTextureHandle streamTextureHandle) const
    {
        LOG_DEBUG(CONTEXT_RENDERER, "ResourceCachedScene::setRenderableResourcesDirtyByStreamTexture(): state change for stream texture :" << streamTextureHandle.asMemoryHandle()
                  << " with source id :" << getStreamTexture(streamTextureHandle).source);

        const UInt32 totalTextureSamplerCount = getTextureSamplerCount();
        for (TextureSamplerHandle samplerHandle(0u); samplerHandle < totalTextureSamplerCount; ++samplerHandle)
        {
            if (isTextureSamplerAllocated(samplerHandle))
            {
                const TextureSampler& sampler = getTextureSampler(samplerHandle);
                if (sampler.contentType == TextureSampler::ContentType::StreamTexture && streamTextureHandle.asMemoryHandle() == sampler.contentHandle)
                    setRenderableResourcesDirtyByTextureSampler(samplerHandle);
            }
        }
    }

    Bool ResourceCachedScene::doesDataInstanceReferToDirtyTextureSampler(DataInstanceHandle handle) const
    {
        if (isDataInstanceAllocated(handle))
        {
            const DataLayoutHandle dataLayoutHandle = getLayoutOfDataInstance(handle);
            const DataLayout& dataLayout = getDataLayout(dataLayoutHandle);
            const UInt32 totalFieldCount = dataLayout.getFieldCount();
            for (DataFieldHandle dataField(0u); dataField < totalFieldCount; ++dataField)
            {
                const EDataType dataType = dataLayout.getField(dataField).dataType;
                if (EDataType_TextureSampler == dataType)
                {
                    const TextureSamplerHandle textureSamplerHandle = getDataTextureSamplerHandle(handle, dataField);
                    if (textureSamplerHandle.isValid() && isTextureSamplerDirty(textureSamplerHandle))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void ResourceCachedScene::resetResourceCache()
    {
        for (RenderableHandle renderable(0u); renderable < getRenderableCount(); ++renderable)
        {
            if (isRenderableAllocated(renderable))
            {
                m_renderableResourcesDirty[renderable.asMemoryHandle()] = true;
            }
        }

        ramses_foreach(m_effectDeviceHandleCache, it)
        {
            *it = DeviceResourceHandle::Invalid();
        }

        ramses_foreach(m_deviceHandleCacheForVertexAttributes, it)
        {
            ramses_foreach(*it, it2)
            {
                *it2 = DeviceResourceHandle::Invalid();
            }
        }

        ramses_foreach(m_deviceHandleCacheForTextures, it)
        {
            *it = DeviceResourceHandle::Invalid();
        }

        ramses_foreach(m_renderTargetCache, it)
        {
            *it = DeviceResourceHandle::Invalid();
        }

        ramses_foreach(m_blitPassCache, it)
        {
            *it = DeviceResourceHandle::Invalid();
        }

        m_renderTargetsDirty = !m_renderTargetCache.empty();
        m_blitPassesDirty = !m_blitPassCache.empty();
    }

}
