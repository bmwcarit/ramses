//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/ActionCollectingScene.h"

namespace ramses_internal
{
    ActionCollectingScene::ActionCollectingScene(const SceneInfo& sceneInfo)
        : ResourceChangeCollectingScene(sceneInfo)
        , m_collection(20000, 512)
        , m_creator(m_collection)
    {
    }

    void ActionCollectingScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        ResourceChangeCollectingScene::preallocateSceneSize(sizeInfo);
        m_creator.preallocateSceneSize(sizeInfo);
    }

    void ActionCollectingScene::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        ResourceChangeCollectingScene::setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor);
        m_creator.setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor);
    }

    void ActionCollectingScene::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        ResourceChangeCollectingScene::setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
        m_creator.setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
    }

    void ActionCollectingScene::setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        ResourceChangeCollectingScene::setDataReference(containerHandle, field, dataRef);
        m_creator.setDataReference(containerHandle, field, dataRef);
    }

    void ActionCollectingScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data)
    {
        ResourceChangeCollectingScene::setDataVector4iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector4iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data)
    {
        ResourceChangeCollectingScene::setDataMatrix22fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix22fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data)
    {
        ResourceChangeCollectingScene::setDataMatrix33fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix33fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data)
    {
        ResourceChangeCollectingScene::setDataMatrix44fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix44fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data)
    {
        ResourceChangeCollectingScene::setDataVector3iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector3iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data)
    {
        ResourceChangeCollectingScene::setDataVector2iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector2iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        ResourceChangeCollectingScene::setDataIntegerArray(containerHandle, field, elementCount, data);
        m_creator.setDataIntegerArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data)
    {
        ResourceChangeCollectingScene::setDataVector4fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector4fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data)
    {
        ResourceChangeCollectingScene::setDataVector3fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector3fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data)
    {
        ResourceChangeCollectingScene::setDataVector2fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector2fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data)
    {
        ResourceChangeCollectingScene::setDataFloatArray(containerHandle, field, elementCount, data);
        m_creator.setDataFloatArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::releaseDataInstance(DataInstanceHandle containerHandle)
    {
        ResourceChangeCollectingScene::releaseDataInstance(containerHandle);
        m_creator.releaseDataInstance(containerHandle);
    }

    DataInstanceHandle ActionCollectingScene::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle)
    {
        DataInstanceHandle handle = ResourceChangeCollectingScene::allocateDataInstance(finishedLayoutHandle, instanceHandle);
        m_creator.allocateDataInstance(finishedLayoutHandle, handle);

        return handle;
    }

    void ActionCollectingScene::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        ResourceChangeCollectingScene::releaseDataLayout(layoutHandle);
        m_creator.releaseDataLayout(layoutHandle);
    }

    DataLayoutHandle ActionCollectingScene::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        DataLayoutHandle handleActual = ResourceChangeCollectingScene::allocateDataLayout(dataFields, handle);
        m_creator.allocateDataLayout(dataFields, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::setScaling(TransformHandle handle, const Vector3& scaling)
    {
        ResourceChangeCollectingScene::setScaling(handle, scaling);
        m_creator.setTransformComponent(ETransformPropertyType_Scaling, handle, scaling);
    }

    void ActionCollectingScene::setRotation(TransformHandle handle, const Vector3& rotation)
    {
        ResourceChangeCollectingScene::setRotation(handle, rotation);
        m_creator.setTransformComponent(ETransformPropertyType_Rotation, handle, rotation);
    }

    void ActionCollectingScene::setTranslation(TransformHandle handle, const Vector3& translation)
    {
        ResourceChangeCollectingScene::setTranslation(handle, translation);
        m_creator.setTransformComponent(ETransformPropertyType_Translation, handle, translation);
    }

    void ActionCollectingScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        ResourceChangeCollectingScene::removeChildFromNode(parent, child);
        m_creator.removeChildFromNode(parent, child);
    }

    void ActionCollectingScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        ResourceChangeCollectingScene::addChildToNode(parent, child);
        m_creator.addChildToNode(parent, child);
    }

    void ActionCollectingScene::releaseTransform(TransformHandle transform)
    {
        ResourceChangeCollectingScene::releaseTransform(transform);
        m_creator.releaseTransform(transform);
    }

    TransformHandle ActionCollectingScene::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        const TransformHandle handleActual = ResourceChangeCollectingScene::allocateTransform(nodeHandle, handle);
        m_creator.allocateTransform(nodeHandle, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseNode(NodeHandle nodeHandle)
    {
        ResourceChangeCollectingScene::releaseNode(nodeHandle);
        m_creator.releaseNode(nodeHandle);
    }

    NodeHandle ActionCollectingScene::allocateNode(UInt32 childrenCount, NodeHandle handle)
    {
        NodeHandle handleActual = ResourceChangeCollectingScene::allocateNode(childrenCount, handle);
        m_creator.allocateNode(childrenCount, handleActual);
        return handleActual;
    }

    CameraHandle ActionCollectingScene::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle)
    {
        CameraHandle handleActual = ResourceChangeCollectingScene::allocateCamera(type, nodeHandle, viewportDataInstance, handle);
        m_creator.allocateCamera(type, nodeHandle, viewportDataInstance, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseCamera(CameraHandle cameraHandle)
    {
        ResourceChangeCollectingScene::releaseCamera(cameraHandle);
        m_creator.releaseCamera(cameraHandle);
    }

    void ActionCollectingScene::setCameraFrustum(CameraHandle cameraHandle, const Frustum& frustum)
    {
        ResourceChangeCollectingScene::setCameraFrustum(cameraHandle, frustum);
        m_creator.setCameraFrustum(cameraHandle, frustum);
    }

    void ActionCollectingScene::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        ResourceChangeCollectingScene::setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
        m_creator.setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
    }

    void ActionCollectingScene::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask)
    {
        ResourceChangeCollectingScene::setRenderStateStencilFunc(stateHandle, func, ref, mask);
        m_creator.setRenderStateStencilFunc(stateHandle, func, ref, mask);
    }

    void ActionCollectingScene::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        ResourceChangeCollectingScene::setRenderStateDepthWrite(stateHandle, flag);
        m_creator.setRenderStateDepthWrite(stateHandle, flag);
    }

    void ActionCollectingScene::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        ResourceChangeCollectingScene::setRenderStateDepthFunc(stateHandle, func);
        m_creator.setRenderStateDepthFunc(stateHandle, func);
    }

    void ActionCollectingScene::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        ResourceChangeCollectingScene::setRenderStateScissorTest(stateHandle, flag, region);
        m_creator.setRenderStateScissorTest(stateHandle, flag, region);
    }

    void ActionCollectingScene::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        if (ResourceChangeCollectingScene::getRenderState(stateHandle).cullMode != cullMode)
        {
            ResourceChangeCollectingScene::setRenderStateCullMode(stateHandle, cullMode);
            m_creator.setRenderStateCullMode(stateHandle, cullMode);
        }
    }

    void ActionCollectingScene::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        if (ResourceChangeCollectingScene::getRenderState(stateHandle).drawMode != drawMode)
        {
            ResourceChangeCollectingScene::setRenderStateDrawMode(stateHandle, drawMode);
            m_creator.setRenderStateDrawMode(stateHandle, drawMode);
        }
    }

    void ActionCollectingScene::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        ResourceChangeCollectingScene::setRenderStateBlendOperations(stateHandle, operationColor, operationAlpha);
        m_creator.setRenderStateBlendOperations(stateHandle, operationColor, operationAlpha);
    }

    void ActionCollectingScene::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        const RenderState& rs = ResourceChangeCollectingScene::getRenderState(stateHandle);
        const EBlendFactor dstA = rs.blendFactorDstAlpha;
        const EBlendFactor srcA = rs.blendFactorSrcAlpha;
        const EBlendFactor dstC = rs.blendFactorDstColor;
        const EBlendFactor srcC = rs.blendFactorSrcColor;
        if (dstA != destAlpha || srcA != srcAlpha || dstC != destColor || srcC != srcColor)
        {
            ResourceChangeCollectingScene::setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
            m_creator.setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
        }
    }

    void ActionCollectingScene::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        const ColorWriteMask previousMask = ResourceChangeCollectingScene::getRenderState(stateHandle).colorWriteMask;
        if (colorMask != previousMask)
        {
            ResourceChangeCollectingScene::setRenderStateColorWriteMask(stateHandle, colorMask);
            m_creator.setRenderStateColorWriteMask(stateHandle, colorMask);
        }
    }

    void ActionCollectingScene::releaseRenderState(RenderStateHandle stateHandle)
    {
        ResourceChangeCollectingScene::releaseRenderState(stateHandle);
        m_creator.releaseRenderState(stateHandle);
    }

    RenderStateHandle ActionCollectingScene::allocateRenderState(RenderStateHandle stateHandle)
    {
        RenderStateHandle handle = ResourceChangeCollectingScene::allocateRenderState(stateHandle);
        m_creator.allocateRenderState(handle);

        return handle;
    }

    void ActionCollectingScene::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        ResourceChangeCollectingScene::setRenderableRenderState(renderableHandle, stateHandle);
        m_creator.setRenderableRenderState(renderableHandle, stateHandle);
    }

    void ActionCollectingScene::setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount)
    {
        if (ResourceChangeCollectingScene::getRenderable(renderableHandle).indexCount != indexCount)
        {
            ResourceChangeCollectingScene::setRenderableIndexCount(renderableHandle, indexCount);
            m_creator.setRenderableIndexCount(renderableHandle, indexCount);
        }
    }

    void ActionCollectingScene::setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex)
    {
        if (ResourceChangeCollectingScene::getRenderable(renderableHandle).startIndex != startIndex)
        {
            ResourceChangeCollectingScene::setRenderableStartIndex(renderableHandle, startIndex);
            m_creator.setRenderableStartIndex(renderableHandle, startIndex);
        }
    }

    void ActionCollectingScene::setRenderableVisibility(RenderableHandle renderableHandle, Bool visibility)
    {
        if (ResourceChangeCollectingScene::getRenderable(renderableHandle).isVisible != visibility)
        {
            ResourceChangeCollectingScene::setRenderableVisibility(renderableHandle, visibility);
            m_creator.setRenderableVisibility(renderableHandle, visibility);
        }
    }

    void ActionCollectingScene::setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount)
    {
        ResourceChangeCollectingScene::setRenderableInstanceCount(renderableHandle, instanceCount);
        m_creator.setRenderableInstanceCount(renderableHandle, instanceCount);
    }

    void ActionCollectingScene::setRenderableDataInstanceAndStateAndEffect(RenderableHandle renderableHandle, DataInstanceHandle newDataInstance, RenderStateHandle stateHandle, const ResourceContentHash& effectHash)
    {
        ResourceChangeCollectingScene::setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, newDataInstance);
        ResourceChangeCollectingScene::setRenderableRenderState(renderableHandle, stateHandle);
        ResourceChangeCollectingScene::setRenderableEffect(renderableHandle, effectHash);
        m_creator.compoundRenderableEffectData(renderableHandle, newDataInstance, stateHandle, effectHash);
    }

    void ActionCollectingScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        ResourceChangeCollectingScene::setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        m_creator.setRenderableDataInstance(renderableHandle, slot, newDataInstance);
    }

    void ActionCollectingScene::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        ResourceChangeCollectingScene::setRenderableEffect(renderableHandle, effectHash);
        m_creator.setRenderableEffect(renderableHandle, effectHash);
    }

    void ActionCollectingScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        ResourceChangeCollectingScene::releaseRenderable(renderableHandle);
        m_creator.releaseRenderable(renderableHandle);
    }

    RenderableHandle ActionCollectingScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        const RenderableHandle handleActual = ResourceChangeCollectingScene::allocateRenderable(nodeHandle, handle);
        m_creator.allocateRenderable(nodeHandle, handleActual);

        return handleActual;
    }

    RenderGroupHandle ActionCollectingScene::allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle)
    {
        const RenderGroupHandle handleActual = ResourceChangeCollectingScene::allocateRenderGroup(renderableCount, nestedGroupCount, groupHandle);
        m_creator.allocateRenderGroup(renderableCount, nestedGroupCount, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        ResourceChangeCollectingScene::releaseRenderGroup(groupHandle);
        m_creator.releaseRenderGroup(groupHandle);
    }

    void ActionCollectingScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order)
    {
        ResourceChangeCollectingScene::addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        m_creator.addRenderableToRenderGroup(groupHandle, renderableHandle, order);
    }

    void ActionCollectingScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order)
    {
        ResourceChangeCollectingScene::addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
        m_creator.addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
    }

    void ActionCollectingScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        ResourceChangeCollectingScene::removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        m_creator.removeRenderableFromRenderGroup(groupHandle, renderableHandle);
    }

    void ActionCollectingScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        ResourceChangeCollectingScene::removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
        m_creator.removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
    }

    AnimationSystemHandle ActionCollectingScene::addAnimationSystem(IAnimationSystem* animationSystem, AnimationSystemHandle externalHandle)
    {
        auto handle = ResourceChangeCollectingScene::addAnimationSystem(animationSystem, externalHandle);
        m_creator.addAnimationSystem(handle, animationSystem->getFlags(), animationSystem->getTotalSizeInformation());
        return handle;
    }

    void ActionCollectingScene::removeAnimationSystem(AnimationSystemHandle animSystemHandle)
    {
        // SceneAction must be created first because animationSystemID is deleted with next call!
        m_creator.removeAnimationSystem(animSystemHandle);
        ResourceChangeCollectingScene::removeAnimationSystem(animSystemHandle);
    }

    ramses_internal::RenderPassHandle ActionCollectingScene::allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle handle /*= InvalidRenderPassHandle*/)
    {
        const RenderPassHandle handleActual = ResourceChangeCollectingScene::allocateRenderPass(renderGroupCount, handle);
        m_creator.allocateRenderPass(renderGroupCount, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderPass(RenderPassHandle handle)
    {
        ResourceChangeCollectingScene::releaseRenderPass(handle);
        m_creator.releaseRenderPass(handle);
    }

    void ActionCollectingScene::setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle)
    {
        ResourceChangeCollectingScene::setRenderPassCamera(passHandle, cameraHandle);
        m_creator.setRenderPassCamera(passHandle, cameraHandle);
    }

    void ActionCollectingScene::setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle)
    {
        ResourceChangeCollectingScene::setRenderPassRenderTarget(passHandle, targetHandle);
        m_creator.setRenderPassRenderTarget(passHandle, targetHandle);
    }

    void ActionCollectingScene::setRenderPassRenderOrder(RenderPassHandle passHandle, Int32 renderOrder)
    {
        ResourceChangeCollectingScene::setRenderPassRenderOrder(passHandle, renderOrder);
        m_creator.setRenderPassRenderOrder(passHandle, renderOrder);
    }

    void ActionCollectingScene::setRenderPassEnabled(RenderPassHandle passHandle, Bool isEnabled)
    {
        ResourceChangeCollectingScene::setRenderPassEnabled(passHandle, isEnabled);
        m_creator.setRenderPassEnabled(passHandle, isEnabled);
    }

    void ActionCollectingScene::setRenderPassRenderOnce(RenderPassHandle passHandle, Bool enable)
    {
        ResourceChangeCollectingScene::setRenderPassRenderOnce(passHandle, enable);
        m_creator.setRenderPassRenderOnce(passHandle, enable);
    }

    void ActionCollectingScene::retriggerRenderPassRenderOnce(RenderPassHandle passHandle)
    {
        ResourceChangeCollectingScene::retriggerRenderPassRenderOnce(passHandle);
        m_creator.retriggerRenderPassRenderOnce(passHandle);
    }

    void ActionCollectingScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order)
    {
        ResourceChangeCollectingScene::addRenderGroupToRenderPass(passHandle, groupHandle, order);
        m_creator.addRenderGroupToRenderPass(passHandle, groupHandle, order);
    }

    void ActionCollectingScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        ResourceChangeCollectingScene::removeRenderGroupFromRenderPass(passHandle, groupHandle);
        m_creator.removeRenderGroupFromRenderPass(passHandle, groupHandle);
    }

    BlitPassHandle ActionCollectingScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle handleActual = ResourceChangeCollectingScene::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_creator.allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        ResourceChangeCollectingScene::releaseBlitPass(passHandle);
        m_creator.releaseBlitPass(passHandle);
    }

    void ActionCollectingScene::setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder)
    {
        ResourceChangeCollectingScene::setBlitPassRenderOrder(passHandle, renderOrder);
        m_creator.setBlitPassRenderOrder(passHandle, renderOrder);
    }

    void ActionCollectingScene::setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled)
    {
        ResourceChangeCollectingScene::setBlitPassEnabled(passHandle, isEnabled);
        m_creator.setBlitPassEnabled(passHandle, isEnabled);
    }

    void ActionCollectingScene::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        ResourceChangeCollectingScene::setBlitPassRegions(passHandle, sourceRegion, destinationRegion);
        m_creator.setBlitPassRegions(passHandle, sourceRegion, destinationRegion);
    }

    DataSlotHandle ActionCollectingScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle /*= DataSlotHandle::Invalid()*/)
    {
        const DataSlotHandle handleActual = ResourceChangeCollectingScene::allocateDataSlot(dataSlot, handle);
        m_creator.allocateDataSlot(dataSlot, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseDataSlot(DataSlotHandle handle)
    {
        ResourceChangeCollectingScene::releaseDataSlot(handle);
        m_creator.releaseDataSlot(handle);
    }

    void ActionCollectingScene::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        ResourceChangeCollectingScene::setDataSlotTexture(handle, texture);
        m_creator.setDataSlotTexture(handle, texture);
    }

    TextureSamplerHandle ActionCollectingScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        const TextureSamplerHandle handleActual = ResourceChangeCollectingScene::allocateTextureSampler(sampler, handle);
        m_creator.allocateTextureSampler(sampler, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        ResourceChangeCollectingScene::releaseTextureSampler(handle);
        m_creator.releaseTextureSampler(handle);
    }

    // Render targets
    RenderTargetHandle ActionCollectingScene::allocateRenderTarget(RenderTargetHandle targetHandle)
    {
        const RenderTargetHandle handleActual = ResourceChangeCollectingScene::allocateRenderTarget(targetHandle);
        m_creator.allocateRenderTarget(handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderTarget  (RenderTargetHandle targetHandle)
    {
        ResourceChangeCollectingScene::releaseRenderTarget( targetHandle );
        m_creator.releaseRenderTarget(targetHandle);
    }

    RenderBufferHandle ActionCollectingScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle handleActual = ResourceChangeCollectingScene::allocateRenderBuffer(renderBuffer, handle);
        m_creator.allocateRenderBuffer(renderBuffer, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        ResourceChangeCollectingScene::releaseRenderBuffer(handle);
        m_creator.releaseRenderBuffer(handle);
    }

    void ActionCollectingScene::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        ResourceChangeCollectingScene::addRenderTargetRenderBuffer(targetHandle, bufferHandle);
        m_creator.addRenderTargetRenderBuffer(targetHandle, bufferHandle);
    }

    void ActionCollectingScene::setRenderPassClearColor(RenderPassHandle pass, const Vector4& clearColor)
    {
        ResourceChangeCollectingScene::setRenderPassClearColor(pass, clearColor);
        m_creator.setRenderPassClearColor(pass, clearColor);
    }

    void ActionCollectingScene::setRenderPassClearFlag(RenderPassHandle pass, UInt32 clearFlag)
    {
        ResourceChangeCollectingScene::setRenderPassClearFlag(pass, clearFlag);
        m_creator.setRenderPassClearFlag(pass, clearFlag);
    }

    StreamTextureHandle ActionCollectingScene::allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle /*= StreamTextureHandle::Invalid()*/)
    {
        const StreamTextureHandle handleActual = ResourceChangeCollectingScene::allocateStreamTexture(streamSource, fallbackTextureHash, streamTextureHandle);
        m_creator.allocateStreamTexture(streamSource, fallbackTextureHash, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseStreamTexture(StreamTextureHandle streamTextureHandle)
    {
        ResourceChangeCollectingScene::releaseStreamTexture(streamTextureHandle);
        m_creator.releaseStreamTexture(streamTextureHandle);
    }

    void ActionCollectingScene::setForceFallbackImage(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage)
    {
        ResourceChangeCollectingScene::setForceFallbackImage(streamTextureHandle, forceFallbackImage);
        m_creator.setStreamTextureForceFallback(streamTextureHandle, forceFallbackImage);
    }

    DataBufferHandle ActionCollectingScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle allocatedHandle = ResourceChangeCollectingScene::allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, handle);
        m_creator.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, allocatedHandle);

        return allocatedHandle;
    }

    void ActionCollectingScene::releaseDataBuffer(DataBufferHandle handle)
    {
        ResourceChangeCollectingScene::releaseDataBuffer(handle);
        m_creator.releaseDataBuffer(handle);
    }

    void ActionCollectingScene::updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data)
    {
        ResourceChangeCollectingScene::updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        m_creator.updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
    }

    TextureBufferHandle ActionCollectingScene::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        const TextureBufferHandle allocatedHandle = ResourceChangeCollectingScene::allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        m_creator.allocateTextureBuffer(textureFormat, mipMapDimensions, allocatedHandle);

        return allocatedHandle;
    }

    void ActionCollectingScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        ResourceChangeCollectingScene::releaseTextureBuffer(handle);
        m_creator.releaseTextureBuffer(handle);
    }

    void ActionCollectingScene::updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data)
    {
        ResourceChangeCollectingScene::updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        const UInt32 dataSize = width * height * GetTexelSizeFromFormat(getTextureBuffer(handle).textureFormat);
        m_creator.updateTextureBuffer(handle, mipLevel, x, y, width, height, data, dataSize);
    }

    const SceneActionCollection& ActionCollectingScene::getSceneActionCollection() const
    {
        return m_collection;
    }

    SceneActionCollection& ActionCollectingScene::getSceneActionCollection()
    {
        return m_collection;
    }
}
