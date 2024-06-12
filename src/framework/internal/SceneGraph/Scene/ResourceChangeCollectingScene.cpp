//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/ResourceChangeCollectingScene.h"
#include "internal/Core/Utils/MemoryPoolExplicit.h"

namespace ramses::internal
{
    ResourceChangeCollectingScene::ResourceChangeCollectingScene(const SceneInfo& sceneInfo)
        : BaseT(sceneInfo)
    {
    }

    const SceneResourceActionVector& ResourceChangeCollectingScene::getSceneResourceActions() const
    {
        return m_sceneResourceActions;
    }

    bool ResourceChangeCollectingScene::haveResourcesChanged() const
    {
        return m_resourcesChanged;
    }

    void ResourceChangeCollectingScene::resetResourceChanges()
    {
        m_sceneResourceActions.clear();
        m_resourcesChanged = false;
    }


    void ResourceChangeCollectingScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        m_resourcesChanged = true;
        BaseT::releaseRenderable(renderableHandle);

    }

    void ResourceChangeCollectingScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        m_resourcesChanged = true;
        BaseT::setRenderableDataInstance(renderableHandle, slot, newDataInstance);
    }

    void ResourceChangeCollectingScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility)
    {
        auto oldVisibility = getRenderable(renderableHandle).visibilityMode;
        if (oldVisibility != visibility && (oldVisibility == EVisibilityMode::Off || visibility == EVisibilityMode::Off))
            m_resourcesChanged = true;

        BaseT::setRenderableVisibility(renderableHandle, visibility);
    }

    void ResourceChangeCollectingScene::setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        m_resourcesChanged = true;
        BaseT::setDataResource(dataInstanceHandle, field, hash, dataBuffer, instancingDivisor, offsetWithinElementInBytes, stride);
    }

    void ResourceChangeCollectingScene::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        m_resourcesChanged = true;
        BaseT::setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
    }

    TextureSamplerHandle ResourceChangeCollectingScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle /*= TextureSamplerHandle::Invalid()*/)
    {
        if (sampler.textureResource.isValid())
            m_resourcesChanged = true;

        return BaseT::allocateTextureSampler(sampler, handle);
    }

    void ResourceChangeCollectingScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        if (getTextureSampler(handle).textureResource.isValid())
            m_resourcesChanged = true;

        BaseT::releaseTextureSampler(handle);
    }

    DataSlotHandle ResourceChangeCollectingScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle /*= DataSlotHandle::Invalid()*/)
    {
        if (dataSlot.attachedTexture.isValid())
            m_resourcesChanged = true;
        return BaseT::allocateDataSlot(dataSlot, handle);
    }

    void ResourceChangeCollectingScene::setDataSlotTexture(DataSlotHandle providerHandle, const ResourceContentHash& texture)
    {
        m_resourcesChanged = true;
        BaseT::setDataSlotTexture(providerHandle, texture);
    }

    void ResourceChangeCollectingScene::releaseDataSlot(DataSlotHandle handle)
    {
        const ResourceContentHash& textureHash = getDataSlot(handle).attachedTexture;
        if (textureHash.isValid())
            m_resourcesChanged = true;

        BaseT::releaseDataSlot(handle);
    }

    UniformBufferHandle ResourceChangeCollectingScene::allocateUniformBuffer(uint32_t size, UniformBufferHandle handle)
    {
        const auto newHandle = BaseT::allocateUniformBuffer(size, handle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateUniformBuffer });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseUniformBuffer(UniformBufferHandle handle)
    {
        BaseT::releaseUniformBuffer(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyUniformBuffer });
    }

    void ResourceChangeCollectingScene::updateUniformBuffer(UniformBufferHandle handle, uint32_t offset, uint32_t size, const std::byte* data)
    {
        BaseT::updateUniformBuffer(handle, offset, size, data);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateUniformBuffer });
    }

    RenderTargetHandle ResourceChangeCollectingScene::allocateRenderTarget(RenderTargetHandle handle)
    {
        const RenderTargetHandle newHandle = BaseT::allocateRenderTarget(handle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderTarget });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseRenderTarget(RenderTargetHandle handle)
    {
        BaseT::releaseRenderTarget(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyRenderTarget });
    }

    RenderBufferHandle ResourceChangeCollectingScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle newHandle = BaseT::allocateRenderBuffer(renderBuffer, handle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderBuffer });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        BaseT::releaseRenderBuffer(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyRenderBuffer });
    }

    void ResourceChangeCollectingScene::setRenderBufferProperties(RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        BaseT::setRenderBufferProperties(handle, width, height, sampleCount);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateRenderBufferProperties });
        m_resourcesChanged = true;
    }

    BlitPassHandle ResourceChangeCollectingScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle newHandle = BaseT::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateBlitPass });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseBlitPass(BlitPassHandle handle)
    {
        BaseT::releaseBlitPass(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyBlitPass });
    }

    DataBufferHandle ResourceChangeCollectingScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle newHandle = BaseT::allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, handle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateDataBuffer });
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseDataBuffer(DataBufferHandle handle)
    {
        BaseT::releaseDataBuffer(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyDataBuffer });
    }

    void ResourceChangeCollectingScene::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        BaseT::updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateDataBuffer });
    }

    TextureBufferHandle ResourceChangeCollectingScene::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        const TextureBufferHandle newHandle = BaseT::allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        m_sceneResourceActions.push_back({ newHandle.asMemoryHandle(), ESceneResourceAction_CreateTextureBuffer});
        return newHandle;
    }

    void ResourceChangeCollectingScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        BaseT::releaseTextureBuffer(handle);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_DestroyTextureBuffer });
    }

    void ResourceChangeCollectingScene::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        BaseT::updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        m_sceneResourceActions.push_back({ handle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer });
    }
}
