//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/ResourceChangeCollectingScene.h"
#include "SceneUtils/ResourceChangeUtils.h"
#include "Utils/MemoryPoolExplicit.h"

namespace ramses_internal
{
    ResourceChangeCollectingScene::ResourceChangeCollectingScene(const SceneInfo& sceneInfo)
        : TransformationCachedScene(sceneInfo)
    {
    }

    void ResourceChangeCollectingScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        this->handleClientResourceReferenceChange(TransformationCachedScene::getRenderable(renderableHandle).effectResource, ResourceContentHash::Invalid());
        TransformationCachedScene::releaseRenderable(renderableHandle);
    }

    void ResourceChangeCollectingScene::releaseDataInstance(DataInstanceHandle dataInstanceHandle)
    {
        const DataLayoutHandle layoutHandle = this->getLayoutOfDataInstance(dataInstanceHandle);
        const DataLayout& layout = this->getDataLayout(layoutHandle);
        const UInt32 bufferCount = layout.getFieldCount();
        for (DataFieldHandle field(0); field < bufferCount; ++field)
        {
            switch (layout.getField(field).dataType)
            {
            case EDataType_Indices:
            case EDataType_UInt16Buffer:
            case EDataType_FloatBuffer:
            case EDataType_Vector2Buffer:
            case EDataType_Vector3Buffer:
            case EDataType_Vector4Buffer:
            {
                const ResourceField& dataResource =  this->getDataResource(dataInstanceHandle, field);
                this->handleClientResourceReferenceChange(dataResource.hash, ResourceContentHash::Invalid());
                break;
            }
            default:
                break;
            }
        }

        TransformationCachedScene::releaseDataInstance(dataInstanceHandle);
    }

    void ResourceChangeCollectingScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        this->handleClientResourceReferenceChange(TransformationCachedScene::getTextureSampler(handle).textureResource, ResourceContentHash::Invalid());
        TransformationCachedScene::releaseTextureSampler(handle);
    }

    void ResourceChangeCollectingScene::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        this->handleClientResourceReferenceChange(TransformationCachedScene::getRenderable(renderableHandle).effectResource, effectHash);
        TransformationCachedScene::setRenderableEffect(renderableHandle, effectHash);
    }

    void ResourceChangeCollectingScene::setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        {
            const ResourceField& dataResource =  TransformationCachedScene::getDataResource(dataInstanceHandle, field);
            this->handleClientResourceReferenceChange(dataResource.hash, hash);
        }
        TransformationCachedScene::setDataResource(dataInstanceHandle, field, hash, dataBuffer, instancingDivisor);
    }

    TextureSamplerHandle ResourceChangeCollectingScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        this->handleClientResourceReferenceChange(ResourceContentHash::Invalid(), sampler.textureResource);
        return TransformationCachedScene::allocateTextureSampler(sampler, handle);
    }

    const SceneResourceChanges& ResourceChangeCollectingScene::getResourceChanges() const
    {
        return m_changes;
    }

    void ResourceChangeCollectingScene::clearResourceChanges()
    {
        m_changes.clear();
    }

    void ResourceChangeCollectingScene::handleClientResourceReferenceChange(const ResourceContentHash& currentHash, const ResourceContentHash& newHash)
    {
        if (newHash != currentHash)
        {
            if (currentHash.isValid())
            {
                this->decrementClientResourceUsageCount(currentHash);
            }

            if (newHash.isValid())
            {
                this->incrementClientResourceUsageCount(newHash);
            }
        }
    }

    void ResourceChangeCollectingScene::incrementClientResourceUsageCount(const ResourceContentHash& hash)
    {
        auto refIt = m_clientResourcesUsageMap.find(hash);
        if (refIt == m_clientResourcesUsageMap.end())
        {
            m_clientResourcesUsageMap.put(hash, 1u);
            ResourceChangeUtils::ConsolidateResource(m_changes.m_addedClientResourceRefs, m_changes.m_removedClientResourceRefs, hash);
        }
        else
        {
            ++refIt->value;
        }
    }

    void ResourceChangeCollectingScene::decrementClientResourceUsageCount(const ResourceContentHash& hash)
    {
        assert(m_clientResourcesUsageMap.contains(hash));
        UInt32& refCount = *m_clientResourcesUsageMap.get(hash);
        assert(refCount > 0u);
        --refCount;

        if (refCount == 0u)
        {
            m_clientResourcesUsageMap.remove(hash);
            ResourceChangeUtils::ConsolidateResource(m_changes.m_removedClientResourceRefs, m_changes.m_addedClientResourceRefs, hash);
        }
    }

    RenderTargetHandle ResourceChangeCollectingScene::allocateRenderTarget(RenderTargetHandle handle)
    {
        const RenderTargetHandle newHandle = TransformationCachedScene::allocateRenderTarget(handle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderTarget });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseRenderTarget(RenderTargetHandle handle)
    {
        TransformationCachedScene::releaseRenderTarget(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyRenderTarget });
    }

    RenderBufferHandle ResourceChangeCollectingScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle newHandle = TransformationCachedScene::allocateRenderBuffer(renderBuffer, handle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderBuffer });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        TransformationCachedScene::releaseRenderBuffer(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyRenderBuffer });
    }

    StreamTextureHandle ResourceChangeCollectingScene::allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle)
    {
        this->handleClientResourceReferenceChange(ResourceContentHash::Invalid(), fallbackTextureHash);
        StreamTextureHandle newHandle = TransformationCachedScene::allocateStreamTexture(streamSource, fallbackTextureHash, streamTextureHandle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateStreamTexture });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseStreamTexture(StreamTextureHandle handle)
    {
        this->handleClientResourceReferenceChange(TransformationCachedScene::getStreamTexture(handle).fallbackTexture, ResourceContentHash::Invalid());
        TransformationCachedScene::releaseStreamTexture(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyStreamTexture });
    }

    DataSlotHandle ResourceChangeCollectingScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        if (dataSlot.attachedTexture.isValid())
        {
            this->incrementClientResourceUsageCount(dataSlot.attachedTexture);
        }

        return TransformationCachedScene::allocateDataSlot(dataSlot, handle);
    }

    void ResourceChangeCollectingScene::releaseDataSlot(DataSlotHandle handle)
    {
        const ResourceContentHash& textureHash = getDataSlot(handle).attachedTexture;
        if (textureHash.isValid())
        {
            this->decrementClientResourceUsageCount(textureHash);
        }

        TransformationCachedScene::releaseDataSlot(handle);
    }

    BlitPassHandle ResourceChangeCollectingScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle newHandle = TransformationCachedScene::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateBlitPass });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseBlitPass(BlitPassHandle handle)
    {
        TransformationCachedScene::releaseBlitPass(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyBlitPass });
    }

    void ResourceChangeCollectingScene::setDataSlotTexture(DataSlotHandle providerHandle, const ResourceContentHash& texture)
    {
        const ramses_internal::ResourceContentHash& currentTextureHash = getDataSlot(providerHandle).attachedTexture;
        assert(currentTextureHash.isValid());
        this->handleClientResourceReferenceChange(currentTextureHash, texture);

        TransformationCachedScene::setDataSlotTexture(providerHandle, texture);
    }

    DataBufferHandle ResourceChangeCollectingScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle newHandle = TransformationCachedScene::allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, handle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateDataBuffer });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseDataBuffer(DataBufferHandle handle)
    {
        TransformationCachedScene::releaseDataBuffer(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyDataBuffer });
    }

    void ResourceChangeCollectingScene::updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data)
    {
        TransformationCachedScene::updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateDataBuffer });
    }

    TextureBufferHandle ResourceChangeCollectingScene::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        const TextureBufferHandle newHandle = TransformationCachedScene::allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        m_changes.m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateTextureBuffer});
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        TransformationCachedScene::releaseTextureBuffer(handle);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyTextureBuffer });
    }

    void ResourceChangeCollectingScene::updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data)
    {
        TransformationCachedScene::updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        m_changes.m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer });
    }
}
