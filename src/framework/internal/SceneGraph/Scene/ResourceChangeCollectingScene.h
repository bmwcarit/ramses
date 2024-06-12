//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/TransformationCachedScene.h"
#include "internal/SceneGraph/Scene/ResourceChanges.h"

namespace ramses::internal
{
    class ResourceChangeCollectingScene : public TransformationCachedScene
    {
        using BaseT = TransformationCachedScene;
    public:
        explicit ResourceChangeCollectingScene(const SceneInfo& sceneInfo = SceneInfo());

        [[nodiscard]] const SceneResourceActionVector&    getSceneResourceActions() const;
        [[nodiscard]] bool                                haveResourcesChanged() const;
        void                                resetResourceChanges();

        // functions which affect client resources
        void                        releaseRenderable(RenderableHandle renderableHandle) override;
        void                        setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility) override;


        void                        setDataResource(DataInstanceHandle dataInstanceHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride) override;
        void                        setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;

        TextureSamplerHandle        allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle) override;
        void                        releaseTextureSampler(TextureSamplerHandle handle) override;

        DataSlotHandle              allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle) override;
        void                        setDataSlotTexture(DataSlotHandle providerHandle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot(DataSlotHandle handle) override;

        // functions which affect scene resources
        UniformBufferHandle         allocateUniformBuffer(uint32_t size, UniformBufferHandle handle) override;
        void                        releaseUniformBuffer(UniformBufferHandle uniformBufferHandle) override;
        void                        updateUniformBuffer(UniformBufferHandle uniformBufferHandle, uint32_t offset, uint32_t size, const std::byte* data) override;

        RenderTargetHandle          allocateRenderTarget(RenderTargetHandle targetHandle) override;
        void                        releaseRenderTarget(RenderTargetHandle handle) override;

        RenderBufferHandle          allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle) override;
        void                        releaseRenderBuffer(RenderBufferHandle handle) override;
        void                        setRenderBufferProperties(RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount) override;

        BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle) override;
        void                        releaseBlitPass(BlitPassHandle handle) override;

        DataBufferHandle            allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle) override;
        void                        releaseDataBuffer(DataBufferHandle handle) override;
        void                        updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data) override;

        TextureBufferHandle         allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle) override;
        void                        releaseTextureBuffer(TextureBufferHandle handle) override;
        void                        updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data) override;

    private:

        SceneResourceActionVector   m_sceneResourceActions;
        bool                        m_resourcesChanged = false;
    };
}
