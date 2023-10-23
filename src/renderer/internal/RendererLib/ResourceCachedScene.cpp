//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/ResourceCachedScene.h"
#include "internal/RendererLib/IResourceDeviceHandleAccessor.h"
#include "internal/RendererLib/TextureLinkCachedScene.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
{
    ResourceCachedScene::ResourceCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : TextureLinkCachedScene(sceneLinksManager, sceneInfo)
    {
    }

    template <typename T>
    void resizeContainerIfSmaller(T& container, uint32_t newSize)
    {
        if (newSize > container.size())
        {
            container.resize(newSize);
        }
    }

    void ResourceCachedScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        TextureLinkCachedScene::preallocateSceneSize(sizeInfo);

        resizeContainerIfSmaller(m_renderableResourcesDirty, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_dataInstancesDirty, sizeInfo.datainstanceCount);
        resizeContainerIfSmaller(m_textureSamplersDirty, sizeInfo.textureSamplerCount);
        resizeContainerIfSmaller(m_effectDeviceHandleCache, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_renderableVertexArrayDirty, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_vertexArrayCache, sizeInfo.renderableCount);
        resizeContainerIfSmaller(m_deviceHandleCacheForTextures, sizeInfo.textureSamplerCount);
        resizeContainerIfSmaller(m_renderTargetCache, sizeInfo.renderTargetCount);
        resizeContainerIfSmaller(m_blitPassCache, sizeInfo.blitPassCount * 2u);
    }

    RenderableHandle ResourceCachedScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        const RenderableHandle renderable = TextureLinkCachedScene::allocateRenderable(nodeHandle, handle);

        const uint32_t indexIntoCache = renderable.asMemoryHandle();
        assert(indexIntoCache < m_effectDeviceHandleCache.size());
        m_effectDeviceHandleCache[indexIntoCache] = DeviceResourceHandle::Invalid();
        setRenderableResourcesDirtyFlag(renderable, true);
        setRenderableVertexArrayDirtyFlag(renderable, true);
        return renderable;
    }

    void ResourceCachedScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        TextureLinkCachedScene::releaseRenderable(renderableHandle);
        setRenderableResourcesDirtyFlag(renderableHandle, false);
        setRenderableVertexArrayDirtyFlag(renderableHandle, true);
    }

    void ResourceCachedScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility)
    {
        // make sure resources get updated if switching from off to other state
        if (getRenderable(renderableHandle).visibilityMode == EVisibilityMode::Off && visibility != EVisibilityMode::Off)
        {
            setRenderableResourcesDirtyFlag(renderableHandle, true);
            setRenderableVertexArrayDirtyFlag(renderableHandle, true);
        }
        TextureLinkCachedScene::setRenderableVisibility(renderableHandle, visibility);
    }

    void ResourceCachedScene::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        TextureLinkCachedScene::setRenderableStartVertex(renderableHandle, startVertex);
        setRenderableVertexArrayDirtyFlag(renderableHandle, true);
    }

    DataInstanceHandle ResourceCachedScene::allocateDataInstance(DataLayoutHandle handle, DataInstanceHandle instanceHandle)
    {
        const DataInstanceHandle dataInstance = TextureLinkCachedScene::allocateDataInstance(handle, instanceHandle);
        setDataInstanceDirtyFlag(dataInstance, true);

        return dataInstance;
    }

    void ResourceCachedScene::releaseDataInstance(DataInstanceHandle dataInstanceHandle)
    {
        TextureLinkCachedScene::releaseDataInstance(dataInstanceHandle);
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    TextureSamplerHandle ResourceCachedScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        const TextureSamplerHandle actualHandle = TextureLinkCachedScene::allocateTextureSampler(sampler, handle);

        const uint32_t indexIntoCache = actualHandle.asMemoryHandle();
        assert(indexIntoCache < m_deviceHandleCacheForTextures.size());
        m_deviceHandleCacheForTextures[indexIntoCache] = DeviceResourceHandle::Invalid();
        setTextureSamplerDirtyFlag(actualHandle, true);

        return actualHandle;
    }

    void ResourceCachedScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        setTextureSamplerDirtyFlag(handle, true);
        TextureLinkCachedScene::releaseTextureSampler(handle);
    }

    void ResourceCachedScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        TextureLinkCachedScene::setRenderableDataInstance(renderableHandle, slot, newDataInstance);

        const uint32_t indexIntoCache = renderableHandle.asMemoryHandle();
        assert(indexIntoCache < m_effectDeviceHandleCache.size());
        m_effectDeviceHandleCache[indexIntoCache] = DeviceResourceHandle::Invalid();

        setRenderableResourcesDirtyFlag(renderableHandle, true);
        setRenderableVertexArrayDirtyFlag(renderableHandle, true);
    }

    void ResourceCachedScene::setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        TextureLinkCachedScene::setDataResource(dataInstanceHandle, field, hash, dataBuffer, instancingDivisor, offsetWithinElementInBytes, stride);
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    void ResourceCachedScene::setDataTextureSamplerHandle(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        TextureLinkCachedScene::setDataTextureSamplerHandle(dataInstanceHandle, field, samplerHandle);
        setDataInstanceDirtyFlag(dataInstanceHandle, true);
    }

    RenderTargetHandle ResourceCachedScene::allocateRenderTarget(RenderTargetHandle targetHandle)
    {
        const RenderTargetHandle rtHandle = TextureLinkCachedScene::allocateRenderTarget(targetHandle);

        const uint32_t indexIntoCache = rtHandle.asMemoryHandle();
        assert(indexIntoCache < m_renderTargetCache.size());
        m_renderTargetCache[indexIntoCache] = DeviceResourceHandle::Invalid();
        m_renderTargetsDirty = true;

        return rtHandle;
    }

    BlitPassHandle ResourceCachedScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        const BlitPassHandle blitPassHandle = TextureLinkCachedScene::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);

        const uint32_t indexIntoCache = blitPassHandle.asMemoryHandle() * 2u;
        assert(indexIntoCache + 1u < m_blitPassCache.size());
        m_blitPassCache[indexIntoCache]      = DeviceResourceHandle::Invalid();
        m_blitPassCache[indexIntoCache + 1u] = DeviceResourceHandle::Invalid();
        m_blitPassesDirty = true;

        return blitPassHandle;
    }

    bool ResourceCachedScene::renderableResourcesDirty(RenderableHandle handle) const
    {
        uint32_t renderableAsIndex = handle.asMemoryHandle();
        return m_renderableResourcesDirty[renderableAsIndex];
    }

    bool ResourceCachedScene::renderableResourcesDirty(const RenderableVector& handles) const
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
        const uint32_t renderableAsIndex = renderable.asMemoryHandle();
        assert(renderableAsIndex < m_effectDeviceHandleCache.size());
        return m_effectDeviceHandleCache[renderableAsIndex];
    }

    const VertexArrayCache& ResourceCachedScene::getCachedHandlesForVertexArrays() const
    {
        return m_vertexArrayCache;
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

    const BoolVector& ResourceCachedScene::getVertexArraysDirtinessFlags() const
    {
        return m_renderableVertexArrayDirty;
    }

    bool ResourceCachedScene::CheckAndUpdateDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, DeviceResourceHandle& deviceHandleInOut, const ResourceContentHash& resourceHash)
    {
        deviceHandleInOut = DeviceResourceHandle::Invalid();
        if (resourceHash.isValid())
            deviceHandleInOut = resourceAccessor.getResourceDeviceHandle(resourceHash);

        return deviceHandleInOut.isValid();
    }

    bool ResourceCachedScene::checkAndUpdateEffectResource(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable)
    {
        const DataInstanceHandle dataInstance = getRenderable(renderable).dataInstances[ERenderableDataSlotType_Geometry];
        ResourceContentHash effectHash = ResourceContentHash::Invalid();
        if (dataInstance.isValid())
        {
            const DataLayoutHandle layoutHandle = getLayoutOfDataInstance(dataInstance);
            effectHash = getDataLayout(layoutHandle).getEffectHash();
        }

        return CheckAndUpdateDeviceHandle(resourceAccessor, m_effectDeviceHandleCache[renderable.asMemoryHandle()], effectHash);
    }

    bool ResourceCachedScene::checkAndUpdateTextureResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable)
    {
        const DataInstanceHandle dataInstance = getRenderable(renderable).dataInstances[ERenderableDataSlotType_Uniforms];
        if (!dataInstance.isValid())
        {
            return false;
        }

        const DataLayoutHandle dataLayoutHandle = getLayoutOfDataInstance(dataInstance);
        const DataLayout& layout = getDataLayout(dataLayoutHandle);
        const uint32_t totalVertexArrayCount = layout.getFieldCount();
        for (DataFieldHandle dataField(0u); dataField < totalVertexArrayCount; ++dataField)
        {
            if (IsTextureSamplerType(layout.getField(dataField).dataType))
            {
                const TextureSamplerHandle sampler = getDataTextureSamplerHandle(dataInstance, dataField);
                if (!sampler.isValid() || !isTextureSamplerAllocated(sampler) ||
                    !updateTextureSamplerResource(resourceAccessor, sampler))
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool ResourceCachedScene::checkGeometryResources(const IResourceDeviceHandleAccessor& resourceAccessor, RenderableHandle renderable)
    {
        const DataInstanceHandle dataInstance = getRenderable(renderable).dataInstances[ERenderableDataSlotType_Geometry];
        if (!dataInstance.isValid())
            return false;

        const DataLayoutHandle geometryLayoutHandle = getLayoutOfDataInstance(dataInstance);
        assert(geometryLayoutHandle.isValid());
        const DataLayout& geometryLayout = getDataLayout(geometryLayoutHandle);

        // there has to be always at least indices field in geometry data layout
        static const DataFieldHandle indicesDataField(0u);
        assert(EFixedSemantics::Indices == geometryLayout.getField(indicesDataField).semantics);
        assert(EDataType::Indices == geometryLayout.getField(indicesDataField).dataType);

        const uint32_t numberOfGeometryFields = geometryLayout.getFieldCount();
        assert(numberOfGeometryFields >= 1u);
        const SceneId sceneId = getSceneId();
        for (DataFieldHandle attributeField = indicesDataField; attributeField < numberOfGeometryFields; ++attributeField)
        {
            assert(IsBufferDataType(geometryLayout.getField(attributeField).dataType));
            const ResourceField& dataResource = getDataResource(dataInstance, attributeField);

            const bool isIndicesField = indicesDataField == attributeField;
            const bool usesIndices = dataResource.hash.isValid() || dataResource.dataBuffer.isValid();

            if (isIndicesField && !usesIndices)
            {
                // Indices field but not using indices -> do not check availability of resource data for this field
                continue;
            }

            DeviceResourceHandle deviceHandle;
            if (dataResource.hash.isValid())
            {
                deviceHandle = resourceAccessor.getResourceDeviceHandle(dataResource.hash);
            }
            else if (dataResource.dataBuffer.isValid())
            {
                deviceHandle = resourceAccessor.getDataBufferDeviceHandle(dataResource.dataBuffer, sceneId);
            }

            if (!deviceHandle.isValid())
                return false;
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

        const auto rtCount = static_cast<uint32_t>(m_renderTargetCache.size());
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

        const auto& blitPasses = getBlitPasses();
        for (const auto& blitPassIt : blitPasses)
        {
            const auto handle = blitPassIt.first;
            const uint32_t indexIntoCache = handle.asMemoryHandle() * 2u;
            DeviceResourceHandle& deviceHandleSrc = m_blitPassCache[indexIntoCache];
            DeviceResourceHandle& deviceHandleDst = m_blitPassCache[indexIntoCache + 1u];
            if ((!deviceHandleSrc.isValid() || !deviceHandleDst.isValid()))
            {
                resourceAccessor.getBlitPassRenderTargetsDeviceHandle(handle, sceneId, deviceHandleSrc, deviceHandleDst);
                assert(deviceHandleSrc.isValid());
                assert(deviceHandleDst.isValid());
            }
        }

        m_blitPassesDirty = false;
    }

    bool ResourceCachedScene::updateTextureSamplerResource(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler)
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
        case TextureSampler::ContentType::RenderBufferMS:
            return updateTextureSamplerResourceAsRenderBuffer(resourceAccessor, RenderBufferHandle(samplerData.contentHandle), m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::OffscreenBuffer:
            m_deviceHandleCacheForTextures[sampler.asMemoryHandle()] = resourceAccessor.getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle(samplerData.contentHandle));
            return true;
        case TextureSampler::ContentType::StreamBuffer:
            return updateTextureSamplerResourceAsStreamBuffer(resourceAccessor, StreamBufferHandle{ samplerData.contentHandle }, getFallbackTextureSampler(sampler), m_deviceHandleCacheForTextures[sampler.asMemoryHandle()]);
        case TextureSampler::ContentType::ExternalTexture:
        {
            if (samplerData.contentHandle == InvalidMemoryHandle)
            {
                m_deviceHandleCacheForTextures[sampler.asMemoryHandle()] = resourceAccessor.getEmptyExternalBufferDeviceHandle();
            }
            else
            {
                m_deviceHandleCacheForTextures[sampler.asMemoryHandle()] = resourceAccessor.getExternalBufferDeviceHandle(ExternalBufferHandle{samplerData.contentHandle});
            }

            assert(m_deviceHandleCacheForTextures[sampler.asMemoryHandle()].isValid());
            return true;
        }
        case TextureSampler::ContentType::None:
            break;
        }

        assert(false);
        return false;
    }

    bool ResourceCachedScene::updateTextureSamplerResourceAsRenderBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut)
    {
        const DeviceResourceHandle textureDeviceHandle = resourceAccessor.getRenderTargetBufferDeviceHandle(bufferHandle, getSceneId());

        deviceHandleOut = textureDeviceHandle;
        return textureDeviceHandle.isValid();
    }

    bool ResourceCachedScene::updateTextureSamplerResourceAsTextureBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const TextureBufferHandle bufferHandle, DeviceResourceHandle& deviceHandleOut)
    {
        const DeviceResourceHandle textureDeviceHandle = resourceAccessor.getTextureBufferDeviceHandle(bufferHandle, getSceneId());

        deviceHandleOut = textureDeviceHandle;
        return textureDeviceHandle.isValid();
    }

    bool ResourceCachedScene::updateTextureSamplerResourceAsStreamBuffer(const IResourceDeviceHandleAccessor& resourceAccessor, const StreamBufferHandle streamBuffer, const TextureSampler& fallbackSamplerData, DeviceResourceHandle& deviceHandleInOut)
    {
        const auto compositedTexture = resourceAccessor.getStreamBufferDeviceHandle(streamBuffer);
        if(compositedTexture.isValid())
        {
            deviceHandleInOut = compositedTexture;
            return true;
        }

        switch (fallbackSamplerData.contentType)
        {
        case TextureSampler::ContentType::ClientTexture:
            return CheckAndUpdateDeviceHandle(resourceAccessor, deviceHandleInOut, fallbackSamplerData.textureResource);
        case TextureSampler::ContentType::TextureBuffer:
            return updateTextureSamplerResourceAsTextureBuffer(resourceAccessor, TextureBufferHandle(fallbackSamplerData.contentHandle), deviceHandleInOut);
        case TextureSampler::ContentType::RenderBuffer:
        case TextureSampler::ContentType::RenderBufferMS:
        case TextureSampler::ContentType::OffscreenBuffer:
        case TextureSampler::ContentType::StreamBuffer:
        case TextureSampler::ContentType::ExternalTexture:
        case TextureSampler::ContentType::None:
            break;
        }

        assert(false);
        return false;
    }

    bool ResourceCachedScene::resolveTextureSamplerResourceDeviceHandle(const IResourceDeviceHandleAccessor& resourceAccessor, TextureSamplerHandle sampler, DeviceResourceHandle& deviceHandleInOut)
    {
        const ResourceContentHash& hash = getTextureSampler(sampler).textureResource;
        return CheckAndUpdateDeviceHandle(resourceAccessor, deviceHandleInOut, hash);
    }

    void ResourceCachedScene::updateRenderableResources(const IResourceDeviceHandleAccessor& resourceAccessor)
    {
        updateRenderablesResourcesDirtiness();

        for (const auto& renderableIt : getRenderables())
        {
            const auto renderable = renderableIt.first;
            const uint32_t renderableAsIndex = renderable.asMemoryHandle();
            if (m_renderableResourcesDirty[renderableAsIndex] && renderableIt.second->visibilityMode != EVisibilityMode::Off)
            {
                if (checkAndUpdateEffectResource(resourceAccessor, renderable) &&
                    checkAndUpdateTextureResources(resourceAccessor, renderable) &&
                    checkGeometryResources(resourceAccessor, renderable))
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
            const uint32_t totalDataInstanceCount = getDataInstanceCount();
            for (DataInstanceHandle d(0u); d < totalDataInstanceCount; ++d)
            {
                if (!isDataInstanceDirty(d) && doesDataInstanceReferToDirtyTextureSampler(d))
                {
                    setDataInstanceDirtyFlag(d, true);
                }
            }

            const uint32_t totalRenderableCount = getRenderableCount();
            for (RenderableHandle r(0u); r < totalRenderableCount; ++r)
            {
                if(isRenderableAllocated(r))
                {
                    if (doesRenderableReferToDirtyUniforms(r))
                        setRenderableResourcesDirtyFlag(r, true);

                    if (doesRenderableReferToDirtyGeometry(r))
                    {
                        setRenderableResourcesDirtyFlag(r, true);
                        setRenderableVertexArrayDirtyFlag(r, true);
                    }
                }
            }

            for (DataInstanceHandle d(0u); d < totalDataInstanceCount; ++d)
            {
                setDataInstanceDirtyFlag(d, false);
            }

            const uint32_t totalTextureSamplerCount = getTextureSamplerCount();
            for (TextureSamplerHandle t(0u); t < totalTextureSamplerCount; ++t)
            {
                setTextureSamplerDirtyFlag(t, false);
            }

            m_renderableResourcesDirtinessNeedsUpdate = false;
        }
    }

    void ResourceCachedScene::setRenderableResourcesDirtyFlag(RenderableHandle handle, bool dirty) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_renderableResourcesDirty.size());
        m_renderableResourcesDirty[indexIntoCache] = dirty;
    }

    void ResourceCachedScene::setRenderableVertexArrayDirtyFlag(RenderableHandle handle, bool dirty) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_renderableVertexArrayDirty.size());
        m_renderableVertexArrayDirty[indexIntoCache] = dirty;

        m_renderableVertexArraysDirty |= dirty;
    }

    void ResourceCachedScene::setDataInstanceDirtyFlag(DataInstanceHandle handle, bool dirty) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_dataInstancesDirty.size());
        m_dataInstancesDirty[indexIntoCache] = dirty;

        m_renderableResourcesDirtinessNeedsUpdate |= dirty;
    }

    void ResourceCachedScene::setTextureSamplerDirtyFlag(TextureSamplerHandle handle, bool dirty) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_textureSamplersDirty.size());
        m_textureSamplersDirty[indexIntoCache] = dirty;

        m_renderableResourcesDirtinessNeedsUpdate |= dirty;
    }

    bool ResourceCachedScene::doesRenderableReferToDirtyUniforms(RenderableHandle handle) const
    {
        assert(isRenderableAllocated(handle));
        const DataInstanceHandle uniformsDataInstance = getRenderable(handle).dataInstances[ERenderableDataSlotType_Uniforms];
        return uniformsDataInstance.isValid() && isDataInstanceDirty(uniformsDataInstance);
    }

    bool ResourceCachedScene::doesRenderableReferToDirtyGeometry(RenderableHandle handle) const
    {
        assert(isRenderableAllocated(handle));
        const DataInstanceHandle geometryDataInstance = getRenderable(handle).dataInstances[ERenderableDataSlotType_Geometry];
        return geometryDataInstance.isValid() && isDataInstanceDirty(geometryDataInstance);
    }

    bool ResourceCachedScene::isDataInstanceDirty(DataInstanceHandle handle) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_dataInstancesDirty.size());
        return m_dataInstancesDirty[indexIntoCache];
    }

    bool ResourceCachedScene::isTextureSamplerDirty(TextureSamplerHandle handle) const
    {
        const uint32_t indexIntoCache = handle.asMemoryHandle();
        assert(indexIntoCache < m_textureSamplersDirty.size());
        return m_textureSamplersDirty[indexIntoCache];
    }

    bool ResourceCachedScene::IsGeometryDataLayout(const DataLayout& layout)
    {
        // TODO vaclav mark data layout explicitly if used for geometry or uniforms when network protocol can change
        // For now we check if indices field is contained to determine geometry data instance - this field exists even if no indices are actually used
        assert(layout.getFieldCount() > 0u);
        return layout.getField(DataFieldHandle(0u)).dataType == EDataType::Indices;
    }

    void ResourceCachedScene::setRenderableResourcesDirtyByTextureSampler(TextureSamplerHandle textureSamplerHandle) const
    {
        setTextureSamplerDirtyFlag(textureSamplerHandle, true);

        const uint32_t indexIntoCache = textureSamplerHandle.asMemoryHandle();
        assert(indexIntoCache < m_deviceHandleCacheForTextures.size());
        m_deviceHandleCacheForTextures[indexIntoCache] = DeviceResourceHandle::Invalid();
    }

    bool ResourceCachedScene::hasDirtyVertexArrays() const
    {
        return m_renderableVertexArraysDirty;
    }

    bool ResourceCachedScene::isRenderableVertexArrayDirty(RenderableHandle renderable) const
    {
        const uint32_t indexIntoCache = renderable.asMemoryHandle();
        assert(indexIntoCache < m_renderableVertexArrayDirty.size());
        return m_renderableVertexArrayDirty[indexIntoCache];
    }

    void ResourceCachedScene::updateRenderableVertexArrays(const IResourceDeviceHandleAccessor& resourceAccessor, const RenderableVector& renderablesWithUpdatedVertexArrays)
    {
        for (const auto renderableHandle : renderablesWithUpdatedVertexArrays)
        {
            const uint32_t renderableAsIndex = renderableHandle.asMemoryHandle();
            assert(m_renderableVertexArrayDirty[renderableAsIndex]);

            m_vertexArrayCache[renderableAsIndex].deviceHandle = {};
            if (!isRenderableAllocated(renderableHandle))
            {
                setRenderableVertexArrayDirtyFlag(renderableHandle, false);
            }
            else if (!m_renderableResourcesDirty[renderableAsIndex])
            {
                assert(getRenderable(renderableHandle).visibilityMode != EVisibilityMode::Off);
                const auto geometryInstance = getRenderable(renderableHandle).dataInstances[ERenderableDataSlotType_Geometry];

                //indices are always in the 1st field (field Zero)
                const ResourceField& indicesDataResource = getDataResource(geometryInstance, DataFieldHandle{ 0u });
                const bool usesIndices = indicesDataResource.hash.isValid() || indicesDataResource.dataBuffer.isValid();

                m_vertexArrayCache[renderableAsIndex].usesIndexArray = usesIndices;
                m_vertexArrayCache[renderableAsIndex].deviceHandle = resourceAccessor.getVertexArrayDeviceHandle(renderableHandle, getSceneId());

                setRenderableVertexArrayDirtyFlag(renderableHandle, false);
            }
        }
    }

    void ResourceCachedScene::markVertexArraysClean()
    {
        m_renderableVertexArraysDirty = false;
    }

    bool ResourceCachedScene::doesDataInstanceReferToDirtyTextureSampler(DataInstanceHandle handle) const
    {
        if (isDataInstanceAllocated(handle))
        {
            const DataLayoutHandle dataLayoutHandle = getLayoutOfDataInstance(handle);
            const DataLayout& dataLayout = getDataLayout(dataLayoutHandle);
            const uint32_t totalFieldCount = dataLayout.getFieldCount();
            for (DataFieldHandle dataField(0u); dataField < totalFieldCount; ++dataField)
            {
                const EDataType dataType = dataLayout.getField(dataField).dataType;
                if (IsTextureSamplerType(dataType))
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
        const auto& renderables = getRenderables();
        for (const auto& renderableIt : renderables)
        {
            m_renderableResourcesDirty[renderableIt.first.asMemoryHandle()] = true;
            m_renderableVertexArrayDirty[renderableIt.first.asMemoryHandle()] = true;
        }

        std::fill(m_effectDeviceHandleCache.begin(), m_effectDeviceHandleCache.end(), DeviceResourceHandle::Invalid());
        std::fill(m_vertexArrayCache.begin(), m_vertexArrayCache.end(), VertexArrayCacheEntry{});
        std::fill(m_deviceHandleCacheForTextures.begin(), m_deviceHandleCacheForTextures.end(), DeviceResourceHandle::Invalid());
        std::fill(m_renderTargetCache.begin(), m_renderTargetCache.end(), DeviceResourceHandle::Invalid());
        std::fill(m_blitPassCache.begin(), m_blitPassCache.end(), DeviceResourceHandle::Invalid());

        m_renderTargetsDirty = !m_renderTargetCache.empty();
        m_blitPassesDirty = !m_blitPassCache.empty();
    }

}
