//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/Scene.h"

#include "internal/Core/Utils/MemoryPoolExplicit.h"
#include "internal/Core/Utils/MemoryPool.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"
#include "internal/PlatformAbstraction/PlatformMath.h"

namespace ramses::internal
{
    template <template<typename, typename> class MEMORYPOOL>
    SceneT<MEMORYPOOL>::SceneT(const SceneInfo& sceneInfo)
        : m_name(sceneInfo.friendlyName)
        , m_sceneId(sceneInfo.sceneID)
        , m_effectTimeSync(FlushTime::InvalidTimestamp)
    {
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setEffectTimeSync(FlushTime::Clock::time_point t)
    {
        m_effectTimeSync = t;
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderPassHandle SceneT<MEMORYPOOL>::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle)
    {
        const RenderPassHandle actualHandle = m_renderPasses.allocate(handle);
        m_renderPasses.getMemory(actualHandle)->renderGroups.reserve(renderGroupCount);
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderPass(RenderPassHandle handle)
    {
        m_renderPasses.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassClearColor(RenderPassHandle passHandle, const glm::vec4& clearColor)
    {
        m_renderPasses.getMemory(passHandle)->clearColor = clearColor;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassClearFlag(RenderPassHandle passHandle, ClearFlags clearFlag)
    {
        m_renderPasses.getMemory(passHandle)->clearFlags = clearFlag;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle)
    {
        m_renderPasses.getMemory(passHandle)->camera = cameraHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderPassCount() const
    {
        return m_renderPasses.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderOrder(RenderPassHandle passHandle, int32_t renderOrder)
    {
        m_renderPasses.getMemory(passHandle)->renderOrder = renderOrder;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle)
    {
        m_renderPasses.getMemory(passHandle)->renderTarget = targetHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassEnabled(RenderPassHandle passHandle, bool isEnabled)
    {
        m_renderPasses.getMemory(passHandle)->isEnabled = isEnabled;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderOnce(RenderPassHandle passHandle, bool enable)
    {
        m_renderPasses.getMemory(passHandle)->isRenderOnce = enable;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::retriggerRenderPassRenderOnce([[maybe_unused]] RenderPassHandle passHandle)
    {
        assert(m_renderPasses.getMemory(passHandle)->isRenderOnce);
        // implemented on renderer side only in a derived scene
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        RenderPass& rp = *m_renderPasses.getMemory(passHandle);
        assert(!RenderGroupUtils::ContainsRenderGroup(groupHandle, rp));
        rp.renderGroups.push_back({ groupHandle, order });
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        RenderPass& rp = *m_renderPasses.getMemory(passHandle);
        const auto it = RenderGroupUtils::FindRenderGroupEntry(groupHandle, rp);
        assert(it != rp.renderGroups.cend());
        rp.renderGroups.erase(it);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const RenderPass& SceneT<MEMORYPOOL>::getRenderPass(RenderPassHandle passHandle) const
    {
        return *m_renderPasses.getMemory(passHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderTargetHandle SceneT<MEMORYPOOL>::allocateRenderTarget(RenderTargetHandle handle)
    {
        return m_renderTargets.allocate(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderTarget(RenderTargetHandle handle)
    {
        assert(m_renderTargets.isAllocated(handle));
        m_renderTargets.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderTargetCount() const
    {
        return m_renderTargets.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        RenderTarget& renderTarget = *m_renderTargets.getMemory(targetHandle);
        assert(!contains_c(renderTarget.renderBuffers, bufferHandle));
        renderTarget.renderBuffers.push_back(bufferHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const
    {
        return static_cast<uint32_t>(m_renderTargets.getMemory(targetHandle)->renderBuffers.size());
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderBufferHandle SceneT<MEMORYPOOL>::getRenderTargetRenderBuffer(RenderTargetHandle targetHandle, uint32_t bufferIndex) const
    {
        const RenderTarget& renderTarget = *m_renderTargets.getMemory(targetHandle);
        assert(bufferIndex < renderTarget.renderBuffers.size());
        return renderTarget.renderBuffers[bufferIndex];
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderBufferHandle SceneT<MEMORYPOOL>::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle actualHandle = m_renderBuffers.allocate(handle);
        *m_renderBuffers.getMemory(actualHandle) = renderBuffer;
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderBuffer(RenderBufferHandle handle)
    {
        assert(m_renderBuffers.isAllocated(handle));
        m_renderBuffers.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderBufferCount() const
    {
        return m_renderBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const RenderBuffer& SceneT<MEMORYPOOL>::getRenderBuffer(RenderBufferHandle handle) const
    {
        return *m_renderBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataBufferHandle SceneT<MEMORYPOOL>::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle allocatedHandle = m_dataBuffers.allocate(handle);
        GeometryDataBuffer& dataBuffer = *m_dataBuffers.getMemory(allocatedHandle);
        dataBuffer.bufferType = dataBufferType;
        dataBuffer.dataType = dataType;
        dataBuffer.data.resize(maximumSizeInBytes);
        return allocatedHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void ramses::internal::SceneT<MEMORYPOOL>::releaseDataBuffer(DataBufferHandle handle)
    {
        assert(m_dataBuffers.isAllocated(handle));
        m_dataBuffers.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getDataBufferCount() const
    {
        return m_dataBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        GeometryDataBuffer& dataBuffer = *m_dataBuffers.getMemory(handle);
        assert(dataSizeInBytes + offsetInBytes <= dataBuffer.data.size());
        std::byte* const copyDestination = dataBuffer.data.data() + offsetInBytes;
        PlatformMemory::Copy(copyDestination, data, dataSizeInBytes);
        dataBuffer.usedSize = std::max(dataBuffer.usedSize, dataSizeInBytes + offsetInBytes);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const GeometryDataBuffer& SceneT<MEMORYPOOL>::getDataBuffer(DataBufferHandle handle) const
    {
        return *m_dataBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TextureBufferHandle SceneT<MEMORYPOOL>::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        const TextureBufferHandle allocatedHandle = m_textureBuffers.allocate(handle);
        TextureBuffer* textureBuffer = m_textureBuffers.getMemory(allocatedHandle);
        textureBuffer->textureFormat = textureFormat;

        textureBuffer->mipMaps.resize(mipMapDimensions.size());

        const auto texelSize = GetTexelSizeFromFormat(textureFormat);
        for (size_t i = 0u; i < mipMapDimensions.size(); ++i)
        {
            auto& mip = textureBuffer->mipMaps[i];
            const uint32_t mipLevelSize = mipMapDimensions[i].width * mipMapDimensions[i].height * texelSize;

            mip.width = mipMapDimensions[i].width;
            mip.height = mipMapDimensions[i].height;
            mip.data.resize(mipLevelSize);
        }

        return allocatedHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseTextureBuffer(TextureBufferHandle handle)
    {
        assert(m_textureBuffers.isAllocated(handle));
        m_textureBuffers.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getTextureBufferCount() const
    {
        return m_textureBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        TextureBuffer& textureBuffer = *m_textureBuffers.getMemory(handle);
        assert(mipLevel < textureBuffer.mipMaps.size());

        const uint32_t texelSize = GetTexelSizeFromFormat(textureBuffer.textureFormat);
        const uint32_t dataSize = width * height * texelSize;
        auto& mip = textureBuffer.mipMaps[mipLevel];
        const uint32_t mipLevelWidth = mip.width;
        const uint32_t mipLevelHeight = mip.height;

        assert(x + width <= mipLevelWidth);
        assert(y + height <= mipLevelHeight);

        //copy updated part of the texture data in row-major order
        std::byte* const mipLevelDataPtr = mip.data.data();
        if (0u == x && 0u == y && width == mipLevelWidth && height == mipLevelHeight)
        {
            PlatformMemory::Copy(mipLevelDataPtr, data, dataSize);
        }
        else
        {
            const uint32_t dataRowSize = width * texelSize;
            const uint32_t mipLevelRowSize = mipLevelWidth * texelSize;

            std::byte* destinationPtr = mipLevelDataPtr + x * texelSize + y * mipLevelRowSize;
            for (uint32_t i = 0u; i < height; ++i)
            {
                PlatformMemory::Copy(destinationPtr, data, dataRowSize);
                destinationPtr += mipLevelRowSize;
                data += dataRowSize;
            }
        }

        //update utilized region
        const Quad updatedRegion{ int32_t(x), int32_t(y), int32_t(width), int32_t(height) };
        mip.usedRegion = mip.usedRegion.getBoundingQuad(updatedRegion);

        assert(mip.usedRegion.x >= 0 && mip.usedRegion.y >= 0 && mip.usedRegion.width <= int32_t(mipLevelWidth) && mip.usedRegion.height <= int32_t(mipLevelHeight));
    }

    template <template<typename, typename> class MEMORYPOOL>
    const TextureBuffer& SceneT<MEMORYPOOL>::getTextureBuffer(TextureBufferHandle handle) const
    {
        return *m_textureBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataSlotHandle SceneT<MEMORYPOOL>::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        assert(dataSlot.type != EDataSlotType::Undefined);
        const DataSlotHandle allocatedHandle = m_dataSlots.allocate(handle);
        *m_dataSlots.getMemory(allocatedHandle) = dataSlot;
        return allocatedHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseDataSlot(DataSlotHandle handle)
    {
        m_dataSlots.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        m_dataSlots.getMemory(handle)->attachedTexture = texture;
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getDataSlotCount() const
    {
        return m_dataSlots.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const DataSlot& SceneT<MEMORYPOOL>::getDataSlot(DataSlotHandle handle) const
    {
        return *m_dataSlots.getMemory(handle);
    }

    template <template <typename, typename> class MEMORYPOOL>
    PickableObjectHandle SceneT<MEMORYPOOL>::allocatePickableObject(DataBufferHandle     geometryHandle,
                                                                    NodeHandle           nodeHandle,
                                                                    PickableObjectId     id,
                                                                    PickableObjectHandle pickableHandle)
    {
        const PickableObjectHandle pickableObjectHandle = m_pickableObjects.allocate(pickableHandle);
        PickableObject&            pickableObject       = *m_pickableObjects.getMemory(pickableObjectHandle);
        pickableObject.geometryHandle                   = geometryHandle;
        pickableObject.nodeHandle                       = nodeHandle;
        pickableObject.id                               = id;
        return pickableObjectHandle;
    }

    template <template <typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releasePickableObject(PickableObjectHandle pickableHandle)
    {
        m_pickableObjects.release(pickableHandle);
    }

    template <template <typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getPickableObjectCount() const
    {
        return m_pickableObjects.getTotalCount();
    }

    template <template <typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id)
    {
        m_pickableObjects.getMemory(pickableHandle)->id = id;
    }

    template <template <typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle)
    {
        m_pickableObjects.getMemory(pickableHandle)->cameraHandle = cameraHandle;
    }

    template <template <typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled)
    {
        m_pickableObjects.getMemory(pickableHandle)->isEnabled = isEnabled;
    }

    template <template <typename, typename> class MEMORYPOOL>
    const PickableObject& SceneT<MEMORYPOOL>::getPickableObject(PickableObjectHandle pickableHandle) const
    {
        return *m_pickableObjects.getMemory(pickableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    BlitPassHandle SceneT<MEMORYPOOL>::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        const BlitPassHandle blitPassHandle = m_blitPasses.allocate(passHandle);
        BlitPass& blitPass = *m_blitPasses.getMemory(blitPassHandle);
        blitPass.sourceRenderBuffer = sourceRenderBufferHandle;
        blitPass.destinationRenderBuffer = destinationRenderBufferHandle;
        return blitPassHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseBlitPass(BlitPassHandle passHandle)
    {
        m_blitPasses.release(passHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getBlitPassCount() const
    {
        return m_blitPasses.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        m_blitPasses.getMemory(passHandle)->renderOrder = renderOrder;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        m_blitPasses.getMemory(passHandle)->isEnabled = isEnabled;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        BlitPass& blitPass = *m_blitPasses.getMemory(passHandle);
        blitPass.sourceRegion = sourceRegion;
        blitPass.destinationRegion = destinationRegion;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const BlitPass& SceneT<MEMORYPOOL>::getBlitPass(BlitPassHandle passHandle) const
    {
        return *m_blitPasses.getMemory(passHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderStateCount() const
    {
        return m_states.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderStateHandle SceneT<MEMORYPOOL>::allocateRenderState(RenderStateHandle stateHandle)
    {
        RenderStateHandle stateHandleActual = m_states.allocate(stateHandle);
        return stateHandleActual;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderState(RenderStateHandle stateHandle)
    {
        m_states.release(stateHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const RenderState& SceneT<MEMORYPOOL>::getRenderState(RenderStateHandle handle) const
    {
        return *m_states.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        RenderState& state = *m_states.getMemory(stateHandle);
        state.blendFactorSrcColor = srcColor;
        state.blendFactorDstColor = destColor;
        state.blendFactorSrcAlpha = srcAlpha;
        state.blendFactorDstAlpha = destAlpha;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        RenderState& state = *m_states.getMemory(stateHandle);
        state.blendOperationAlpha = operationAlpha;
        state.blendOperationColor = operationColor;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color)
    {
        RenderState& state = *m_states.getMemory(stateHandle);
        state.blendColor = color;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        m_states.getMemory(stateHandle)->cullMode = cullMode;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        m_states.getMemory(stateHandle)->drawMode = drawMode;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        m_states.getMemory(stateHandle)->depthFunc = func;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        m_states.getMemory(stateHandle)->depthWrite = flag;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void ramses::internal::SceneT<MEMORYPOOL>::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        m_states.getMemory(stateHandle)->scissorTest = flag;
        m_states.getMemory(stateHandle)->scissorRegion = region;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        RenderState& state = *m_states.getMemory(stateHandle);
        state.stencilFunc = func;
        state.stencilRefValue = ref;
        state.stencilMask = mask;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        RenderState& state = *m_states.getMemory(stateHandle);
        state.stencilOpFail = sfail;
        state.stencilOpDepthFail = dpfail;
        state.stencilOpDepthPass = dppass;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        m_states.getMemory(stateHandle)->colorWriteMask = colorMask;
    }

    template <template<typename, typename> class MEMORYPOOL>
    TextureSamplerHandle SceneT<MEMORYPOOL>::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        assert(sampler.contentType != TextureSampler::ContentType::None);
        assert(!(sampler.textureResource.isValid() && sampler.contentHandle != InvalidMemoryHandle)); // only one type of content valid
        assert(!(sampler.contentType == TextureSampler::ContentType::ClientTexture && !sampler.textureResource.isValid())); // client texture type must have resource hash valid
        assert(!((sampler.contentType != TextureSampler::ContentType::ClientTexture && sampler.contentType != TextureSampler::ContentType::ExternalTexture)
                && sampler.contentHandle == InvalidMemoryHandle)); // other than client and external texture type must have handle valid

        const TextureSamplerHandle handleActual = m_textureSamplers.allocate(handle);
        *m_textureSamplers.getMemory(handleActual) = sampler;

        return handleActual;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseTextureSampler(TextureSamplerHandle handle)
    {
        m_textureSamplers.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getTextureSamplerCount() const
    {
        return m_textureSamplers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const TextureSampler& SceneT<MEMORYPOOL>::getTextureSampler(TextureSamplerHandle handle) const
    {
        return *m_textureSamplers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TextureSampler& SceneT<MEMORYPOOL>::getTextureSamplerInternal(TextureSamplerHandle handle)
    {
        return *m_textureSamplers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount)
    {
        m_renderables.getMemory(renderableHandle)->indexCount = indexCount;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex)
    {
        m_renderables.getMemory(renderableHandle)->startIndex = startIndex;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        m_renderables.getMemory(renderableHandle)->renderState = stateHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility)
    {
        m_renderables.getMemory(renderableHandle)->visibilityMode = visibility;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount)
    {
        m_renderables.getMemory(renderableHandle)->instanceCount = instanceCount;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        m_renderables.getMemory(renderableHandle)->startVertex = startVertex;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Renderable& SceneT<MEMORYPOOL>::getRenderable(RenderableHandle renderableHandle) const
    {
        return *m_renderables.getMemory(renderableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderGroupHandle SceneT<MEMORYPOOL>::allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle)
    {
        const RenderGroupHandle actualHandle = m_renderGroups.allocate(groupHandle);
        RenderGroup& rg = *m_renderGroups.getMemory(actualHandle);
        rg.renderables.reserve(renderableCount);
        rg.renderGroups.reserve(nestedGroupCount);
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        m_renderGroups.release(groupHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderGroupCount() const
    {
        return m_renderGroups.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        RenderGroup& rg = *m_renderGroups.getMemory(groupHandle);
        assert(!RenderGroupUtils::ContainsRenderable(renderableHandle, rg));
        rg.renderables.push_back({ renderableHandle, order });
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        RenderGroup& rg = *m_renderGroups.getMemory(groupHandle);
        const auto it = RenderGroupUtils::FindRenderableEntry(renderableHandle, rg);
        assert(it != rg.renderables.cend());
        rg.renderables.erase(it);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        RenderGroup& rg = *m_renderGroups.getMemory(groupHandleParent);
        assert(!RenderGroupUtils::ContainsRenderGroup(groupHandleChild, rg));
        rg.renderGroups.push_back({ groupHandleChild, order });
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        RenderGroup& rg = *m_renderGroups.getMemory(groupHandleParent);
        const auto it = RenderGroupUtils::FindRenderGroupEntry(groupHandleChild, rg);
        assert(it != rg.renderGroups.cend());
        rg.renderGroups.erase(it);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const RenderGroup& SceneT<MEMORYPOOL>::getRenderGroup(RenderGroupHandle groupHandle) const
    {
        return *m_renderGroups.getMemory(groupHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        m_renderables.getMemory(renderableHandle)->dataInstances[slot] = newDataInstance;
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataLayoutHandle SceneT<MEMORYPOOL>::getLayoutOfDataInstance(DataInstanceHandle containerHandle) const
    {
        assert(m_dataInstanceMemory.isAllocated(containerHandle));
        return m_dataInstanceMemory.getMemory(containerHandle)->getLayoutHandle();
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getRenderableCount() const
    {
        return m_renderables.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    CameraHandle SceneT<MEMORYPOOL>::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        const CameraHandle actualHandle = m_cameras.allocate(handle);
        Camera& camera = *m_cameras.getMemory(actualHandle);
        camera.projectionType = type;
        camera.node = nodeHandle;
        camera.dataInstance = dataInstance;

        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseCamera(CameraHandle cameraHandle)
    {
        m_cameras.release(cameraHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getCameraCount() const
    {
        return m_cameras.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Camera& SceneT<MEMORYPOOL>::getCamera(CameraHandle cameraHandle) const
    {
        return *m_cameras.getMemory(cameraHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderable(RenderableHandle renderableHandle)
    {
        m_renderables.release(renderableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const std::string& SceneT<MEMORYPOOL>::getName() const
    {
        return m_name;
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getDataInstanceCount() const
    {
        return m_dataInstanceMemory.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getDataLayoutCount() const
    {
        return m_dataLayoutMemory.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const float* data)
    {
        setInstanceDataInternal<float>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::vec2* data)
    {
        setInstanceDataInternal<glm::vec2>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::vec3* data)
    {
        setInstanceDataInternal<glm::vec3>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::vec4* data)
    {
        setInstanceDataInternal<glm::vec4>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::mat2* data)
    {
        setInstanceDataInternal<glm::mat2>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::mat3* data)
    {
        setInstanceDataInternal<glm::mat3>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::mat4* data)
    {
        setInstanceDataInternal<glm::mat4>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const bool* data)
    {
        setInstanceDataInternal<bool>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const int32_t* data)
    {
        setInstanceDataInternal<int32_t>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::ivec2* data)
    {
        setInstanceDataInternal<glm::ivec2>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::ivec3* data)
    {
        setInstanceDataInternal<glm::ivec3>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, uint32_t elementCount, const glm::ivec4* data)
    {
        setInstanceDataInternal<glm::ivec4>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle fieldId, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        //one and only one (xor) of newHashValue and dataBuffer must be valid
        assert(!hash.isValid() ^ !dataBuffer.isValid());

        ResourceField newField;
        newField.hash = hash;
        newField.dataBuffer = dataBuffer;
        newField.instancingDivisor = instancingDivisor;
        newField.offsetWithinElementInBytes = offsetWithinElementInBytes;
        newField.stride = stride;
        setInstanceDataInternal<ResourceField>(containerHandle, fieldId, 1, &newField);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle fieldId, TextureSamplerHandle samplerHandle)
    {
        setInstanceDataInternal<TextureSamplerHandle>(containerHandle, fieldId, 1, &samplerHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataReference(DataInstanceHandle containerHandle, DataFieldHandle fieldId, DataInstanceHandle dataRef)
    {
        setInstanceDataInternal<DataInstanceHandle>(containerHandle, fieldId, 1, &dataRef);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataInstanceHandle SceneT<MEMORYPOOL>::allocateDataInstance(DataLayoutHandle layoutHandle, DataInstanceHandle instanceHandle)
    {
        const DataLayout& layout = *m_dataLayoutMemory.getMemory(layoutHandle);
        const DataInstanceHandle containerHandle = m_dataInstanceMemory.allocate(instanceHandle);

        uint32_t dataInstanceSize = layout.getTotalSize();
        DataInstance* instance = m_dataInstanceMemory.getMemory(containerHandle);
        *instance = DataInstance(layoutHandle, dataInstanceSize);

        // initialize data instance fields
        // TODO violin this can be generalized further, e.g. via templated static inplace contructor
        for (DataFieldHandle i(0u); i < layout.getFieldCount(); ++i)
        {
            const EDataType fieldDataType = layout.getField(i).dataType;
            switch (fieldDataType)
            {
            case EDataType::TextureSampler2D:
            case EDataType::TextureSampler2DMS:
            case EDataType::TextureSamplerExternal:
            case EDataType::TextureSampler3D:
            case EDataType::TextureSamplerCube:
            {
                const TextureSamplerHandle invalid = TextureSamplerHandle::Invalid();
                instance->setTypedData<TextureSamplerHandle>(layout.getFieldOffset(i), 1, &invalid);
                break;
            }
            case EDataType::DataReference:
            {
                DataInstanceHandle invalid;
                instance->setTypedData<DataInstanceHandle>(layout.getFieldOffset(i), 1, &invalid);
                break;
            }
            case EDataType::Indices:
            case EDataType::UInt16Buffer:
            case EDataType::FloatBuffer:
            case EDataType::Vector2Buffer:
            case EDataType::Vector3Buffer:
            case EDataType::Vector4Buffer:
            {
                const ResourceField resourceField{ ResourceContentHash::Invalid(), DataBufferHandle::Invalid(), 0u , 0u, 0u };
                instance->setTypedData<ResourceField>(layout.getFieldOffset(i), 1, &resourceField);
                break;
            }
            default:
                break;
            }
        }

        return containerHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseDataInstance(DataInstanceHandle containerHandle)
    {
        assert(isDataLayoutAllocated(m_dataInstanceMemory.getMemory(containerHandle)->getLayoutHandle()));
        m_dataInstanceMemory.release(containerHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        m_dataLayoutMemory.release(layoutHandle);
    }


    template <template<typename, typename> class MEMORYPOOL>
    DataLayoutHandle SceneT<MEMORYPOOL>::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        const DataLayoutHandle actualHandle = m_dataLayoutMemory.allocate(handle);

        DataLayout& dataLayout = *m_dataLayoutMemory.getMemory(actualHandle);
        dataLayout.setDataFields(dataFields);
        dataLayout.setEffectHash(effectHash);

        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getTransformCount() const
    {
        return m_transforms.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getNodeCount() const
    {
        return m_nodes.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::getParent(NodeHandle nodeHandle) const
    {
        return m_nodes.getMemory(nodeHandle)->parent;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        TopologyNode& parentNode = *m_nodes.getMemory(parent);
        assert(!contains_c(parentNode.children, child));
        parentNode.children.push_back(child);
        m_nodes.getMemory(child)->parent = parent;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        TopologyNode& parentNode = *m_nodes.getMemory(parent);
        const auto childIt = find_c(parentNode.children, child);
        assert(childIt != parentNode.children.end());
        parentNode.children.erase(childIt);
        m_nodes.getMemory(child)->parent = NodeHandle::Invalid();
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getChildCount(NodeHandle parent) const
    {
        return static_cast<uint32_t>(m_nodes.getMemory(parent)->children.size());
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::getChild(NodeHandle parent, uint32_t childNumber) const
    {
        const TopologyNode& parentNode = *m_nodes.getMemory(parent);
        assert(childNumber < parentNode.children.size());
        return parentNode.children[childNumber];
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        m_nodes.preallocateSize(sizeInfo.nodeCount);
        m_cameras.preallocateSize(sizeInfo.cameraCount);
        m_renderables.preallocateSize(sizeInfo.renderableCount);
        m_states.preallocateSize(sizeInfo.renderStateCount);
        m_transforms.preallocateSize(sizeInfo.transformCount);
        m_dataLayoutMemory.preallocateSize(sizeInfo.datalayoutCount);
        m_dataInstanceMemory.preallocateSize(sizeInfo.datainstanceCount);
        m_renderGroups.preallocateSize(sizeInfo.renderGroupCount);
        m_renderPasses.preallocateSize(sizeInfo.renderPassCount);
        m_blitPasses.preallocateSize(sizeInfo.blitPassCount);
        m_renderTargets.preallocateSize(sizeInfo.renderTargetCount);
        m_renderBuffers.preallocateSize(sizeInfo.renderBufferCount);
        m_textureSamplers.preallocateSize(sizeInfo.textureSamplerCount);
        m_dataSlots.preallocateSize(sizeInfo.dataSlotCount);
        m_dataBuffers.preallocateSize(sizeInfo.dataBufferCount);
        m_textureBuffers.preallocateSize(sizeInfo.textureBufferCount);
        m_pickableObjects.preallocateSize(sizeInfo.pickableObjectCount);
        m_sceneReferences.preallocateSize(sizeInfo.sceneReferenceCount);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TransformHandle SceneT<MEMORYPOOL>::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        const TransformHandle actualHandle = m_transforms.allocate(handle);
        m_transforms.getMemory(actualHandle)->node = nodeHandle;
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const TopologyNode& SceneT<MEMORYPOOL>::getNode(NodeHandle handle) const
    {
        return *m_nodes.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::getTransformNode(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->node;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseTransform(TransformHandle transform)
    {
        m_transforms.release(transform);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec3& SceneT<MEMORYPOOL>::getTranslation(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->translation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec4& SceneT<MEMORYPOOL>::getRotation(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->rotation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    ERotationType SceneT<MEMORYPOOL>::getRotationType(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->rotationType;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec3& SceneT<MEMORYPOOL>::getScaling(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->scaling;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setTranslation(TransformHandle handle, const glm::vec3& translation)
    {
        m_transforms.getMemory(handle)->translation = translation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRotation(TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType)
    {
        auto transformMemory = m_transforms.getMemory(handle);
        transformMemory->rotation = rotation;
        transformMemory->rotationType = rotationType;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setScaling(TransformHandle handle, const glm::vec3& scaling)
    {
        m_transforms.getMemory(handle)->scaling = scaling;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const TopologyTransform& SceneT<MEMORYPOOL>::getTransform(TransformHandle handle) const
    {
        return *m_transforms.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        const NodeHandle actualHandle = m_nodes.allocate(handle);
        m_nodes.getMemory(actualHandle)->children.reserve(childrenCount);
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderableHandle SceneT<MEMORYPOOL>::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        const RenderableHandle actualHandle = m_renderables.allocate(handle);
        m_renderables.getMemory(actualHandle)->node = nodeHandle;

        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseNode(NodeHandle nodeHandle)
    {
        m_nodes.release(nodeHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    SceneReferenceHandle SceneT<MEMORYPOOL>::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        const auto actualHandle = m_sceneReferences.allocate(handle);
        m_sceneReferences.getMemory(actualHandle)->sceneId = sceneId;
        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseSceneReference(SceneReferenceHandle handle)
    {
        m_sceneReferences.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state)
    {
        m_sceneReferences.getMemory(handle)->requestedState = state;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable)
    {
        m_sceneReferences.getMemory(handle)->flushNotifications = enable;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder)
    {
        m_sceneReferences.getMemory(handle)->renderOrder = renderOrder;
    }

    template <template<typename, typename> class MEMORYPOOL>
    uint32_t SceneT<MEMORYPOOL>::getSceneReferenceCount() const
    {
        return m_sceneReferences.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const SceneReference& SceneT<MEMORYPOOL>::getSceneReference(SceneReferenceHandle handle) const
    {
        return *m_sceneReferences.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    SceneSizeInformation SceneT<MEMORYPOOL>::getSceneSizeInformation() const
    {
        SceneSizeInformation sizeInfo;
        sizeInfo.nodeCount = m_nodes.getTotalCount();
        sizeInfo.cameraCount = m_cameras.getTotalCount();
        sizeInfo.transformCount = m_transforms.getTotalCount();
        sizeInfo.renderableCount = m_renderables.getTotalCount();
        sizeInfo.renderStateCount = m_states.getTotalCount();
        sizeInfo.datalayoutCount = m_dataLayoutMemory.getTotalCount();
        sizeInfo.datainstanceCount = m_dataInstanceMemory.getTotalCount();
        sizeInfo.renderGroupCount = m_renderGroups.getTotalCount();
        sizeInfo.renderPassCount = m_renderPasses.getTotalCount();
        sizeInfo.blitPassCount = m_blitPasses.getTotalCount();
        sizeInfo.renderTargetCount = m_renderTargets.getTotalCount();
        sizeInfo.renderBufferCount = m_renderBuffers.getTotalCount();
        sizeInfo.textureSamplerCount = m_textureSamplers.getTotalCount();
        sizeInfo.dataSlotCount = m_dataSlots.getTotalCount();
        sizeInfo.dataBufferCount = m_dataBuffers.getTotalCount();
        sizeInfo.textureBufferCount = m_textureBuffers.getTotalCount();
        sizeInfo.pickableObjectCount = m_pickableObjects.getTotalCount();
        sizeInfo.sceneReferenceCount = m_sceneReferences.getTotalCount();
        return sizeInfo;
    }

    // get/setData*Array wrappers for animations

    template <template<typename, typename> class MEMORYPOOL>
    float SceneT<MEMORYPOOL>::getDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataFloatArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec2& SceneT<MEMORYPOOL>::getDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector2fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec3& SceneT<MEMORYPOOL>::getDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector3fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::vec4& SceneT<MEMORYPOOL>::getDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector4fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    bool SceneT<MEMORYPOOL>::getDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataBooleanArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    int32_t SceneT<MEMORYPOOL>::getDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataIntegerArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::mat2& SceneT<MEMORYPOOL>::getDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix22fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::mat3& SceneT<MEMORYPOOL>::getDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix33fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::mat4& SceneT<MEMORYPOOL>::getDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix44fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::ivec2& SceneT<MEMORYPOOL>::getDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector2iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::ivec3& SceneT<MEMORYPOOL>::getDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector3iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const glm::ivec4& SceneT<MEMORYPOOL>::getDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector4iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field, float data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataFloatArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec2& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector2fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec3& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector3fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec4& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector4fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field, bool data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataBooleanArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field, int32_t data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataIntegerArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec2& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector2iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec3& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector3iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec4& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector4iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat2& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix22fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat3& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix33fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat4& data)
    {
        uint32_t elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix44fArray(containerHandle, field, elementCount, &data);
    }

    template class SceneT < MemoryPool >;
    template class SceneT < MemoryPoolExplicit > ;
}
