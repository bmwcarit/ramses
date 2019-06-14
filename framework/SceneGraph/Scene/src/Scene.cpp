//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/Scene.h"

#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

#include "Utils/MemoryPoolExplicit.h"
#include "Utils/MemoryPool.h"
#include "Utils/LogMacros.h"
#include "Utils/StringUtils.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/RenderGroupUtils.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal
{
    constexpr DataFieldHandle Camera::ViewportOffsetField;
    constexpr DataFieldHandle Camera::ViewportSizeField;

    template <template<typename, typename> class MEMORYPOOL>
    SceneT<MEMORYPOOL>::SceneT(const SceneInfo& sceneInfo)
        : m_name(sceneInfo.friendlyName)
        , m_sceneId(sceneInfo.sceneID)
    {
    }

    template <template<typename, typename> class MEMORYPOOL>
    SceneT<MEMORYPOOL>::~SceneT()
    {
        for (auto i = AnimationSystemHandle(0); i < getAnimationSystemCount(); ++i)
        {
            if (isAnimationSystemAllocated(i))
            {
                removeAnimationSystem(i);
            }
        }
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderPassHandle SceneT<MEMORYPOOL>::allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle handle)
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
    void SceneT<MEMORYPOOL>::setRenderPassClearColor(RenderPassHandle passHandle, const Vector4& clearColor)
    {
        m_renderPasses.getMemory(passHandle)->clearColor = clearColor;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassClearFlag(RenderPassHandle passHandle, UInt32 clearFlag)
    {
        m_renderPasses.getMemory(passHandle)->clearFlags = clearFlag;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle)
    {
        m_renderPasses.getMemory(passHandle)->camera = cameraHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getRenderPassCount() const
    {
        return m_renderPasses.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderOrder(RenderPassHandle passHandle, Int32 renderOrder)
    {
        m_renderPasses.getMemory(passHandle)->renderOrder = renderOrder;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle)
    {
        m_renderPasses.getMemory(passHandle)->renderTarget = targetHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassEnabled(RenderPassHandle passHandle, Bool isEnabled)
    {
        m_renderPasses.getMemory(passHandle)->isEnabled = isEnabled;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderPassRenderOnce(RenderPassHandle passHandle, Bool enable)
    {
        m_renderPasses.getMemory(passHandle)->isRenderOnce = enable;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::retriggerRenderPassRenderOnce(RenderPassHandle passHandle)
    {
        UNUSED(passHandle);
        assert(m_renderPasses.getMemory(passHandle)->isRenderOnce);
        // implemented on renderer side only in a derived scene
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order)
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
    UInt32 SceneT<MEMORYPOOL>::getRenderTargetCount() const
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
    UInt32 SceneT<MEMORYPOOL>::getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const
    {
        return static_cast<UInt32>(m_renderTargets.getMemory(targetHandle)->renderBuffers.size());
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderBufferHandle SceneT<MEMORYPOOL>::getRenderTargetRenderBuffer(RenderTargetHandle targetHandle, UInt32 bufferIndex) const
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
    UInt32 SceneT<MEMORYPOOL>::getRenderBufferCount() const
    {
        return m_renderBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const RenderBuffer& SceneT<MEMORYPOOL>::getRenderBuffer(RenderBufferHandle handle) const
    {
        return *m_renderBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    StreamTextureHandle SceneT<MEMORYPOOL>::allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle handle /*= StreamTextureHandle::Invalid()*/)
    {
        const StreamTextureHandle streamTextureHandle = m_streamTextures.allocate(handle);
        StreamTexture* streamTexture = m_streamTextures.getMemory(streamTextureHandle);
        assert(0 != streamTexture);
        streamTexture->fallbackTexture = fallbackTextureHash;
        streamTexture->source = streamSource;
        return streamTextureHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseStreamTexture(StreamTextureHandle streamTextureHandle)
    {
        m_streamTextures.release(streamTextureHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getStreamTextureCount() const
    {
        return m_streamTextures.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setForceFallbackImage(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage)
    {
        m_streamTextures.getMemory(streamTextureHandle)->forceFallbackTexture = forceFallbackImage;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const StreamTexture& SceneT<MEMORYPOOL>::getStreamTexture(StreamTextureHandle streamTextureHandle) const
    {
        return *m_streamTextures.getMemory(streamTextureHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataBufferHandle SceneT<MEMORYPOOL>::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle allocatedHandle = m_dataBuffers.allocate(handle);
        GeometryDataBuffer& dataBuffer = *m_dataBuffers.getMemory(allocatedHandle);
        dataBuffer.bufferType = dataBufferType;
        dataBuffer.dataType = dataType;
        dataBuffer.data.resize(maximumSizeInBytes);
        return allocatedHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void ramses_internal::SceneT<MEMORYPOOL>::releaseDataBuffer(DataBufferHandle handle)
    {
        assert(m_dataBuffers.isAllocated(handle));
        m_dataBuffers.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getDataBufferCount() const
    {
        return m_dataBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data)
    {
        GeometryDataBuffer& dataBuffer = *m_dataBuffers.getMemory(handle);
        assert(dataSizeInBytes + offsetInBytes <= dataBuffer.data.size());
        Byte* const copyDestination = dataBuffer.data.data() + offsetInBytes;
        PlatformMemory::Copy(copyDestination, data, dataSizeInBytes);
        dataBuffer.usedSize = max<UInt32>(dataBuffer.usedSize, dataSizeInBytes + offsetInBytes);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const GeometryDataBuffer& SceneT<MEMORYPOOL>::getDataBuffer(DataBufferHandle handle) const
    {
        return *m_dataBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TextureBufferHandle SceneT<MEMORYPOOL>::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        const TextureBufferHandle allocatedHandle = m_textureBuffers.allocate(handle);
        TextureBuffer* textureBuffer = m_textureBuffers.getMemory(allocatedHandle);
        textureBuffer->textureFormat = textureFormat;

        textureBuffer->mipMaps.resize(mipMapDimensions.size());

        const auto texelSize = GetTexelSizeFromFormat(textureFormat);
        for (UInt32 i = 0u; i < mipMapDimensions.size(); ++i)
        {
            auto& mip = textureBuffer->mipMaps[i];
            const UInt32 mipLevelSize = mipMapDimensions[i].width * mipMapDimensions[i].height * texelSize;

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
    UInt32 SceneT<MEMORYPOOL>::getTextureBufferCount() const
    {
        return m_textureBuffers.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data)
    {
        TextureBuffer& textureBuffer = *m_textureBuffers.getMemory(handle);
        assert(mipLevel < textureBuffer.mipMaps.size());

        const UInt32 texelSize = GetTexelSizeFromFormat(textureBuffer.textureFormat);
        const UInt32 dataSize = width * height * texelSize;
        auto& mip = textureBuffer.mipMaps[mipLevel];
        const UInt32 mipLevelWidth = mip.width;
        const UInt32 mipLevelHeight = mip.height;

        assert(x + width <= mipLevelWidth);
        assert(y + height <= mipLevelHeight);

        //copy updated part of the texture data in row-major order
        Byte* const mipLevelDataPtr = mip.data.data();
        if (0u == x && 0u == y && width == mipLevelWidth && height == mipLevelHeight)
        {
            PlatformMemory::Copy(mipLevelDataPtr, data, dataSize);
        }
        else
        {
            const UInt32 dataRowSize = width * texelSize;
            const UInt32 mipLevelRowSize = mipLevelWidth * texelSize;

            Byte* destinationPtr = mipLevelDataPtr + x * texelSize + y * mipLevelRowSize;
            for (UInt32 i = 0u; i < height; ++i)
            {
                PlatformMemory::Copy(destinationPtr, data, dataRowSize);
                destinationPtr += mipLevelRowSize;
                data += dataRowSize;
            }
        }

        //update utilized region
        const Quad updatedRegion{ Int32(x), Int32(y), Int32(width), Int32(height) };
        mip.usedRegion = mip.usedRegion.getBoundingQuad(updatedRegion);

        assert(mip.usedRegion.x >= 0 && mip.usedRegion.y >= 0 && mip.usedRegion.width <= Int32(mipLevelWidth) && mip.usedRegion.height <= Int32(mipLevelHeight));
    }

    template <template<typename, typename> class MEMORYPOOL>
    const TextureBuffer& SceneT<MEMORYPOOL>::getTextureBuffer(TextureBufferHandle handle) const
    {
        return *m_textureBuffers.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataSlotHandle SceneT<MEMORYPOOL>::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle /*= DataSlotHandle::Invalid()*/)
    {
        assert(dataSlot.type != EDataSlotType_Undefined);
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
    UInt32 SceneT<MEMORYPOOL>::getDataSlotCount() const
    {
        return m_dataSlots.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const DataSlot& SceneT<MEMORYPOOL>::getDataSlot(DataSlotHandle handle) const
    {
        return *m_dataSlots.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    BlitPassHandle SceneT<MEMORYPOOL>::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
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
    ramses_internal::UInt32 SceneT<MEMORYPOOL>::getBlitPassCount() const
    {
        return m_blitPasses.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder)
    {
        m_blitPasses.getMemory(passHandle)->renderOrder = renderOrder;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled)
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
    AnimationSystemHandle SceneT<MEMORYPOOL>::addAnimationSystem(IAnimationSystem* animationSystem, AnimationSystemHandle externalHandle)
    {
        assert(nullptr != animationSystem);
        const auto handle = m_animationSystems.allocate(externalHandle);
        *m_animationSystems.getMemory(handle) = animationSystem;
        animationSystem->setHandle(handle);
        return handle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::removeAnimationSystem(AnimationSystemHandle handle)
    {
        delete getAnimationSystem(handle);
        m_animationSystems.release(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    IAnimationSystem* SceneT<MEMORYPOOL>::getAnimationSystem(AnimationSystemHandle handle)
    {
        // Non-const version of getAnimationSystem cast to its const version to avoid duplicating code
        return const_cast<IAnimationSystem*>((const_cast<const SceneT&>(*this)).getAnimationSystem(handle));
    }

    template <template<typename, typename> class MEMORYPOOL>
    const IAnimationSystem* SceneT<MEMORYPOOL>::getAnimationSystem(AnimationSystemHandle handle) const
    {
        return m_animationSystems.isAllocated(handle) ? *m_animationSystems.getMemory(handle) : nullptr;
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getAnimationSystemCount() const
    {
        return m_animationSystems.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getRenderStateCount() const
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
    void ramses_internal::SceneT<MEMORYPOOL>::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        m_states.getMemory(stateHandle)->scissorTest = flag;
        m_states.getMemory(stateHandle)->scissorRegion = region;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask)
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
        assert(!(sampler.contentType != TextureSampler::ContentType::ClientTexture && sampler.contentHandle == InvalidMemoryHandle)); // other than client texture type must have handle valid

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
    UInt32 SceneT<MEMORYPOOL>::getTextureSamplerCount() const
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
    void SceneT<MEMORYPOOL>::setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount)
    {
        m_renderables.getMemory(renderableHandle)->indexCount = indexCount;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex)
    {
        m_renderables.getMemory(renderableHandle)->startIndex = startIndex;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        m_renderables.getMemory(renderableHandle)->renderState = stateHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableVisibility(RenderableHandle renderableHandle, Bool visible)
    {
        m_renderables.getMemory(renderableHandle)->isVisible = visible;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount)
    {
        m_renderables.getMemory(renderableHandle)->instanceCount = instanceCount;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Renderable& SceneT<MEMORYPOOL>::getRenderable(RenderableHandle renderableHandle) const
    {
        return *m_renderables.getMemory(renderableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderGroupHandle SceneT<MEMORYPOOL>::allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle)
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
    UInt32 SceneT<MEMORYPOOL>::getRenderGroupCount() const
    {
        return m_renderGroups.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order)
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
    void SceneT<MEMORYPOOL>::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order)
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
    void SceneT<MEMORYPOOL>::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        m_renderables.getMemory(renderableHandle)->effectResource = effectHash;
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataLayoutHandle SceneT<MEMORYPOOL>::getLayoutOfDataInstance(DataInstanceHandle containerHandle) const
    {
        assert(m_dataInstanceMemory.isAllocated(containerHandle));
        return m_dataInstanceMemory.getMemory(containerHandle)->getLayoutHandle();
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getRenderableCount() const
    {
        return m_renderables.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    CameraHandle SceneT<MEMORYPOOL>::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle)
    {
        const CameraHandle actualHandle = m_cameras.allocate(handle);
        Camera& camera = *m_cameras.getMemory(actualHandle);
        camera.projectionType = type;
        camera.node = nodeHandle;
        camera.viewportDataInstance = viewportDataInstance;

        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseCamera(CameraHandle cameraHandle)
    {
        m_cameras.release(cameraHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getCameraCount() const
    {
        return m_cameras.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setCameraFrustum(CameraHandle cameraHandle, const Frustum& frustum)
    {
        assert(m_cameras.getMemory(cameraHandle)->projectionType != ECameraProjectionType_Renderer);
        m_cameras.getMemory(cameraHandle)->frustum = frustum;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Camera& SceneT<MEMORYPOOL>::getCamera(CameraHandle cameraHandle) const
    {
        return *m_cameras.getMemory(cameraHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::releaseRenderable(RenderableHandle nodeHandle)
    {
        m_renderables.release(nodeHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const String& SceneT<MEMORYPOOL>::getName() const
    {
        return m_name;
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getDataInstanceCount() const
    {
        return m_dataInstanceMemory.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getDataLayoutCount() const
    {
        return m_dataLayoutMemory.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Float* SceneT<MEMORYPOOL>::getDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Float>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix22f* SceneT<MEMORYPOOL>::getDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix22f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix33f* SceneT<MEMORYPOOL>::getDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix33f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix44f* SceneT<MEMORYPOOL>::getDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix44f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector2* SceneT<MEMORYPOOL>::getDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector2>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3* SceneT<MEMORYPOOL>::getDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector3>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector4* SceneT<MEMORYPOOL>::getDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector4>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Int32* SceneT<MEMORYPOOL>::getDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Int32>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector2i* SceneT<MEMORYPOOL>::getDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector2i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3i* SceneT<MEMORYPOOL>::getDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector3i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector4i* SceneT<MEMORYPOOL>::getDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector4i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    TextureSamplerHandle SceneT<MEMORYPOOL>::getDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return *getInstanceDataInternal<TextureSamplerHandle>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    DataInstanceHandle SceneT<MEMORYPOOL>::getDataReference(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return *getInstanceDataInternal<DataInstanceHandle>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    const ResourceField& SceneT<MEMORYPOOL>::getDataResource(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        const ResourceField* resourceField = getInstanceDataInternal<ResourceField>(containerHandle, fieldId);
        return *resourceField;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Float* newValue)
    {
        setInstanceDataInternal<Float>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector2* newValue)
    {
        setInstanceDataInternal<Vector2>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector3* newValue)
    {
        setInstanceDataInternal<Vector3>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector4* newValue)
    {
        setInstanceDataInternal<Vector4>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Matrix22f* data)
    {
        setInstanceDataInternal<Matrix22f>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Matrix33f* data)
    {
        setInstanceDataInternal<Matrix33f>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Matrix44f* data)
    {
        setInstanceDataInternal<Matrix44f>(containerHandle, fieldId, elementCount, data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Int32* newValue)
    {
        setInstanceDataInternal<Int32>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector2i* newValue)
    {
        setInstanceDataInternal<Vector2i>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector3i* newValue)
    {
        setInstanceDataInternal<Vector3i>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId, UInt32 elementCount, const Vector4i* newValue)
    {
        setInstanceDataInternal<Vector4i>(containerHandle, fieldId, elementCount, newValue);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle fieldId, const ResourceContentHash& newHashValue, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        //one and only one (xor) of newHashValue and dataBuffer must be valid
        assert(!newHashValue.isValid() ^ !dataBuffer.isValid());

        ResourceField newField;
        newField.hash = newHashValue;
        newField.dataBuffer = dataBuffer;
        newField.instancingDivisor = instancingDivisor;
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

        UInt32 dataInstanceSize = layout.getTotalSize();
        DataInstance* instance = m_dataInstanceMemory.getMemory(containerHandle);
        *instance = DataInstance(layoutHandle, dataInstanceSize);

        // initialize data instance fields
        // TODO violin this can be generalized further, e.g. via templated static inplace contructor
        for (DataFieldHandle i(0u); i < layout.getFieldCount(); ++i)
        {
            const EDataType fieldDataType = layout.getField(i).dataType;
            switch (fieldDataType)
            {
            case EDataType_TextureSampler:
            {
                const TextureSamplerHandle invalid = TextureSamplerHandle::Invalid();
                instance->setTypedData<TextureSamplerHandle>(layout.getFieldOffset(i), 1, &invalid);
                break;
            }
            case EDataType_DataReference:
            {
                DataInstanceHandle invalid;
                instance->setTypedData<DataInstanceHandle>(layout.getFieldOffset(i), 1, &invalid);
                break;
            }
            case EDataType_Indices:
            case EDataType_UInt16Buffer:
            case EDataType_FloatBuffer:
            case EDataType_Vector2Buffer:
            case EDataType_Vector3Buffer:
            case EDataType_Vector4Buffer:
            {
                const ResourceField resourceField{ ResourceContentHash::Invalid(), DataBufferHandle::Invalid(), 0u };
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
    DataLayoutHandle SceneT<MEMORYPOOL>::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        const DataLayoutHandle actualHandle = m_dataLayoutMemory.allocate(handle);

        DataLayout& dataLayout = *m_dataLayoutMemory.getMemory(actualHandle);
        dataLayout.setDataFields(dataFields);

        return actualHandle;
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getTransformCount() const
    {
        return m_transforms.getTotalCount();
    }

    template <template<typename, typename> class MEMORYPOOL>
    UInt32 SceneT<MEMORYPOOL>::getNodeCount() const
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
    UInt32 SceneT<MEMORYPOOL>::getChildCount(NodeHandle parent) const
    {
        return static_cast<UInt32>(m_nodes.getMemory(parent)->children.size());
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::getChild(NodeHandle parent, UInt32 childNumber) const
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
        m_streamTextures.preallocateSize(sizeInfo.streamTextureCount);
        m_dataSlots.preallocateSize(sizeInfo.dataSlotCount);
        m_dataBuffers.preallocateSize(sizeInfo.dataBufferCount);
        m_animationSystems.preallocateSize(sizeInfo.animationSystemCount);
        m_textureBuffers.preallocateSize(sizeInfo.textureBufferCount);
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
    const Vector3& SceneT<MEMORYPOOL>::getTranslation(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->translation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3& SceneT<MEMORYPOOL>::getRotation(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->rotation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3& SceneT<MEMORYPOOL>::getScaling(TransformHandle handle) const
    {
        return m_transforms.getMemory(handle)->scaling;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setTranslation(TransformHandle handle, const Vector3& translation)
    {
        m_transforms.getMemory(handle)->translation = translation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setRotation(TransformHandle handle, const Vector3& rotation)
    {
        m_transforms.getMemory(handle)->rotation = rotation;
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setScaling(TransformHandle handle, const Vector3& scaling)
    {
        m_transforms.getMemory(handle)->scaling = scaling;
    }

    template <template<typename, typename> class MEMORYPOOL>
    NodeHandle SceneT<MEMORYPOOL>::allocateNode(UInt32 childrenCount, NodeHandle handle)
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
    SceneSizeInformation SceneT<MEMORYPOOL>::getSceneSizeInformation() const
    {
        SceneSizeInformation sizeInfo;
        sizeInfo.nodeCount = getNodeCount();
        sizeInfo.cameraCount = getCameraCount();
        sizeInfo.transformCount = getTransformCount();
        sizeInfo.renderableCount = getRenderableCount();
        sizeInfo.renderStateCount = getRenderStateCount();
        sizeInfo.datalayoutCount = getDataLayoutCount();
        sizeInfo.datainstanceCount = getDataInstanceCount();
        sizeInfo.renderGroupCount = getRenderGroupCount();
        sizeInfo.renderPassCount = getRenderPassCount();
        sizeInfo.blitPassCount = getBlitPassCount();
        sizeInfo.renderTargetCount = getRenderTargetCount();
        sizeInfo.renderBufferCount = getRenderBufferCount();
        sizeInfo.textureSamplerCount = getTextureSamplerCount();
        sizeInfo.streamTextureCount = getStreamTextureCount();
        sizeInfo.dataSlotCount = getDataSlotCount();
        sizeInfo.dataBufferCount = getDataBufferCount();
        sizeInfo.animationSystemCount = getAnimationSystemCount();
        sizeInfo.textureBufferCount = getTextureBufferCount();

        return sizeInfo;
    }

    // get/setData*Array wrappers for animations

    template <template<typename, typename> class MEMORYPOOL>
    Float SceneT<MEMORYPOOL>::getDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataFloatArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector2& SceneT<MEMORYPOOL>::getDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector2fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3& SceneT<MEMORYPOOL>::getDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector3fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector4& SceneT<MEMORYPOOL>::getDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector4fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    Int32 SceneT<MEMORYPOOL>::getDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataIntegerArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix22f& SceneT<MEMORYPOOL>::getDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix22fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix33f& SceneT<MEMORYPOOL>::getDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix33fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Matrix44f& SceneT<MEMORYPOOL>::getDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataMatrix44fArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector2i& SceneT<MEMORYPOOL>::getDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector2iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector3i& SceneT<MEMORYPOOL>::getDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector3iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    const Vector4i& SceneT<MEMORYPOOL>::getDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        assert(getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount == 1);
        return getDataVector4iArray(containerHandle, field)[0];
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field, Float data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataFloatArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector2fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector3fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector4fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field, Int32 data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataIntegerArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2i& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector2iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3i& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector3iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4i& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataVector4iArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix22f& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix22fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix33f& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix33fArray(containerHandle, field, elementCount, &data);
    }

    template <template<typename, typename> class MEMORYPOOL>
    void SceneT<MEMORYPOOL>::setDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix44f& data)
    {
        UInt32 elementCount = getDataLayout(getLayoutOfDataInstance(containerHandle)).getField(field).elementCount;
        assert(elementCount == 1);
        setDataMatrix44fArray(containerHandle, field, elementCount, &data);
    }

    template class SceneT < MemoryPool >;
    template class SceneT < MemoryPoolExplicit > ;
}
