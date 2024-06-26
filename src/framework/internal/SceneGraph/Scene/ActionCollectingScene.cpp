//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/ActionCollectingScene.h"

namespace ramses::internal
{
    ActionCollectingScene::ActionCollectingScene(const SceneInfo& sceneInfo, EFeatureLevel featureLevel)
        : BaseT(sceneInfo)
        , m_collection(20000, 512)
        , m_creator(m_collection, featureLevel)
    {
    }

    void ActionCollectingScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        BaseT::preallocateSceneSize(sizeInfo);
        m_creator.preallocateSceneSize(sizeInfo);
    }

    void ActionCollectingScene::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        BaseT::setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor, offsetWithinElementInBytes, stride);
        m_creator.setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor, offsetWithinElementInBytes, stride);
    }

    void ActionCollectingScene::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        BaseT::setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
        m_creator.setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
    }

    void ActionCollectingScene::setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        BaseT::setDataReference(containerHandle, field, dataRef);
        m_creator.setDataReference(containerHandle, field, dataRef);
    }

    void ActionCollectingScene::setDataUniformBuffer(DataInstanceHandle containerHandle, DataFieldHandle field, UniformBufferHandle uniformBufferHandle)
    {
        BaseT::setDataUniformBuffer(containerHandle, field, uniformBufferHandle);
        m_creator.setDataUniformBuffer(containerHandle, field, uniformBufferHandle);
    }

    void ActionCollectingScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data)
    {
        BaseT::setDataVector4iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector4iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data)
    {
        BaseT::setDataMatrix22fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix22fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data)
    {
        BaseT::setDataMatrix33fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix33fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data)
    {
        BaseT::setDataMatrix44fArray(containerHandle, field, elementCount, data);
        m_creator.setDataMatrix44fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data)
    {
        BaseT::setDataVector3iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector3iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data)
    {
        BaseT::setDataVector2iArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector2iArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data)
    {
        BaseT::setDataBooleanArray(containerHandle, field, elementCount, data);
        m_creator.setDataBooleanArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data)
    {
        BaseT::setDataIntegerArray(containerHandle, field, elementCount, data);
        m_creator.setDataIntegerArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data)
    {
        BaseT::setDataVector4fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector4fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data)
    {
        BaseT::setDataVector3fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector3fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data)
    {
        BaseT::setDataVector2fArray(containerHandle, field, elementCount, data);
        m_creator.setDataVector2fArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data)
    {
        BaseT::setDataFloatArray(containerHandle, field, elementCount, data);
        m_creator.setDataFloatArray(containerHandle, field, elementCount, data);
    }

    void ActionCollectingScene::releaseDataInstance(DataInstanceHandle containerHandle)
    {
        BaseT::releaseDataInstance(containerHandle);
        m_creator.releaseDataInstance(containerHandle);
    }

    DataInstanceHandle ActionCollectingScene::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle)
    {
        DataInstanceHandle handle = BaseT::allocateDataInstance(finishedLayoutHandle, instanceHandle);
        m_creator.allocateDataInstance(finishedLayoutHandle, handle);

        return handle;
    }

    void ActionCollectingScene::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        BaseT::releaseDataLayout(layoutHandle);
        m_creator.releaseDataLayout(layoutHandle);
    }

    DataLayoutHandle ActionCollectingScene::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        DataLayoutHandle handleActual = BaseT::allocateDataLayout(dataFields, effectHash, handle);
        m_creator.allocateDataLayout(dataFields, effectHash, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::setScaling(TransformHandle handle, const glm::vec3& scaling)
    {
        BaseT::setScaling(handle, scaling);
        m_creator.setScaling(handle, scaling);
    }

    void ActionCollectingScene::setRotation(TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType)
    {
        BaseT::setRotation(handle, rotation, rotationType);
        m_creator.setRotation(handle, rotation, rotationType);
    }

    void ActionCollectingScene::setTranslation(TransformHandle handle, const glm::vec3& translation)
    {
        BaseT::setTranslation(handle, translation);
        m_creator.setTranslation(handle, translation);
    }

    void ActionCollectingScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        BaseT::removeChildFromNode(parent, child);
        m_creator.removeChildFromNode(parent, child);
    }

    void ActionCollectingScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        BaseT::addChildToNode(parent, child);
        m_creator.addChildToNode(parent, child);
    }

    void ActionCollectingScene::releaseTransform(TransformHandle transform)
    {
        BaseT::releaseTransform(transform);
        m_creator.releaseTransform(transform);
    }

    TransformHandle ActionCollectingScene::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        const TransformHandle handleActual = BaseT::allocateTransform(nodeHandle, handle);
        m_creator.allocateTransform(nodeHandle, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseNode(NodeHandle nodeHandle)
    {
        BaseT::releaseNode(nodeHandle);
        m_creator.releaseNode(nodeHandle);
    }

    NodeHandle ActionCollectingScene::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        NodeHandle handleActual = BaseT::allocateNode(childrenCount, handle);
        m_creator.allocateNode(childrenCount, handleActual);
        return handleActual;
    }

    CameraHandle ActionCollectingScene::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        CameraHandle handleActual = BaseT::allocateCamera(type, nodeHandle, dataInstance, handle);
        m_creator.allocateCamera(type, nodeHandle, dataInstance, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseCamera(CameraHandle cameraHandle)
    {
        BaseT::releaseCamera(cameraHandle);
        m_creator.releaseCamera(cameraHandle);
    }

    void ActionCollectingScene::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        BaseT::setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
        m_creator.setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
    }

    void ActionCollectingScene::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        BaseT::setRenderStateStencilFunc(stateHandle, func, ref, mask);
        m_creator.setRenderStateStencilFunc(stateHandle, func, ref, mask);
    }

    void ActionCollectingScene::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        BaseT::setRenderStateDepthWrite(stateHandle, flag);
        m_creator.setRenderStateDepthWrite(stateHandle, flag);
    }

    void ActionCollectingScene::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        BaseT::setRenderStateDepthFunc(stateHandle, func);
        m_creator.setRenderStateDepthFunc(stateHandle, func);
    }

    void ActionCollectingScene::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        BaseT::setRenderStateScissorTest(stateHandle, flag, region);
        m_creator.setRenderStateScissorTest(stateHandle, flag, region);
    }

    void ActionCollectingScene::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        if (BaseT::getRenderState(stateHandle).cullMode != cullMode)
        {
            BaseT::setRenderStateCullMode(stateHandle, cullMode);
            m_creator.setRenderStateCullMode(stateHandle, cullMode);
        }
    }

    void ActionCollectingScene::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        if (BaseT::getRenderState(stateHandle).drawMode != drawMode)
        {
            BaseT::setRenderStateDrawMode(stateHandle, drawMode);
            m_creator.setRenderStateDrawMode(stateHandle, drawMode);
        }
    }

    void ActionCollectingScene::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        BaseT::setRenderStateBlendOperations(stateHandle, operationColor, operationAlpha);
        m_creator.setRenderStateBlendOperations(stateHandle, operationColor, operationAlpha);
    }

    void ActionCollectingScene::setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color)
    {
        BaseT::setRenderStateBlendColor(stateHandle, color);
        m_creator.setRenderStateBlendColor(stateHandle, color);
    }

    void ActionCollectingScene::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        const RenderState& rs = BaseT::getRenderState(stateHandle);
        const EBlendFactor dstA = rs.blendFactorDstAlpha;
        const EBlendFactor srcA = rs.blendFactorSrcAlpha;
        const EBlendFactor dstC = rs.blendFactorDstColor;
        const EBlendFactor srcC = rs.blendFactorSrcColor;
        if (dstA != destAlpha || srcA != srcAlpha || dstC != destColor || srcC != srcColor)
        {
            BaseT::setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
            m_creator.setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
        }
    }

    void ActionCollectingScene::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        const ColorWriteMask previousMask = BaseT::getRenderState(stateHandle).colorWriteMask;
        if (colorMask != previousMask)
        {
            BaseT::setRenderStateColorWriteMask(stateHandle, colorMask);
            m_creator.setRenderStateColorWriteMask(stateHandle, colorMask);
        }
    }

    void ActionCollectingScene::releaseRenderState(RenderStateHandle stateHandle)
    {
        BaseT::releaseRenderState(stateHandle);
        m_creator.releaseRenderState(stateHandle);
    }

    RenderStateHandle ActionCollectingScene::allocateRenderState(RenderStateHandle stateHandle)
    {
        RenderStateHandle handle = BaseT::allocateRenderState(stateHandle);
        m_creator.allocateRenderState(handle);

        return handle;
    }

    void ActionCollectingScene::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        BaseT::setRenderableRenderState(renderableHandle, stateHandle);
        m_creator.setRenderableRenderState(renderableHandle, stateHandle);
    }

    void ActionCollectingScene::setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount)
    {
        if (BaseT::getRenderable(renderableHandle).indexCount != indexCount)
        {
            BaseT::setRenderableIndexCount(renderableHandle, indexCount);
            m_creator.setRenderableIndexCount(renderableHandle, indexCount);
        }
    }

    void ActionCollectingScene::setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex)
    {
        if (BaseT::getRenderable(renderableHandle).startIndex != startIndex)
        {
            BaseT::setRenderableStartIndex(renderableHandle, startIndex);
            m_creator.setRenderableStartIndex(renderableHandle, startIndex);
        }
    }

    void ActionCollectingScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visibility)
    {
        if (BaseT::getRenderable(renderableHandle).visibilityMode != visibility)
        {
            BaseT::setRenderableVisibility(renderableHandle, visibility);
            m_creator.setRenderableVisibility(renderableHandle, visibility);
        }
    }

    void ActionCollectingScene::setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount)
    {
        BaseT::setRenderableInstanceCount(renderableHandle, instanceCount);
        m_creator.setRenderableInstanceCount(renderableHandle, instanceCount);
    }

    void ActionCollectingScene::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        BaseT::setRenderableStartVertex(renderableHandle, startVertex);
        m_creator.setRenderableStartVertex(renderableHandle, startVertex);
    }

    void ActionCollectingScene::setRenderableUniformsDataInstanceAndState(RenderableHandle renderableHandle, DataInstanceHandle newDataInstance, RenderStateHandle stateHandle)
    {
        BaseT::setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, newDataInstance);
        BaseT::setRenderableRenderState(renderableHandle, stateHandle);

        m_creator.compoundRenderableData(renderableHandle, newDataInstance, stateHandle);
    }

    void ActionCollectingScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        BaseT::setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        m_creator.setRenderableDataInstance(renderableHandle, slot, newDataInstance);
    }

    void ActionCollectingScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        BaseT::releaseRenderable(renderableHandle);
        m_creator.releaseRenderable(renderableHandle);
    }

    RenderableHandle ActionCollectingScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        const RenderableHandle handleActual = BaseT::allocateRenderable(nodeHandle, handle);
        m_creator.allocateRenderable(nodeHandle, handleActual);

        return handleActual;
    }

    RenderGroupHandle ActionCollectingScene::allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle)
    {
        const RenderGroupHandle handleActual = BaseT::allocateRenderGroup(renderableCount, nestedGroupCount, groupHandle);
        m_creator.allocateRenderGroup(renderableCount, nestedGroupCount, handleActual);

        return handleActual;
    }

    void ActionCollectingScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        BaseT::releaseRenderGroup(groupHandle);
        m_creator.releaseRenderGroup(groupHandle);
    }

    void ActionCollectingScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        BaseT::addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        m_creator.addRenderableToRenderGroup(groupHandle, renderableHandle, order);
    }

    void ActionCollectingScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        BaseT::addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
        m_creator.addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
    }

    void ActionCollectingScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        BaseT::removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        m_creator.removeRenderableFromRenderGroup(groupHandle, renderableHandle);
    }

    void ActionCollectingScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        BaseT::removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
        m_creator.removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
    }

    RenderPassHandle ActionCollectingScene::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle /*= InvalidRenderPassHandle*/)
    {
        const RenderPassHandle handleActual = BaseT::allocateRenderPass(renderGroupCount, handle);
        m_creator.allocateRenderPass(renderGroupCount, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderPass(RenderPassHandle handle)
    {
        BaseT::releaseRenderPass(handle);
        m_creator.releaseRenderPass(handle);
    }

    void ActionCollectingScene::setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle)
    {
        BaseT::setRenderPassCamera(passHandle, cameraHandle);
        m_creator.setRenderPassCamera(passHandle, cameraHandle);
    }

    void ActionCollectingScene::setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle)
    {
        BaseT::setRenderPassRenderTarget(passHandle, targetHandle);
        m_creator.setRenderPassRenderTarget(passHandle, targetHandle);
    }

    void ActionCollectingScene::setRenderPassRenderOrder(RenderPassHandle passHandle, int32_t renderOrder)
    {
        BaseT::setRenderPassRenderOrder(passHandle, renderOrder);
        m_creator.setRenderPassRenderOrder(passHandle, renderOrder);
    }

    void ActionCollectingScene::setRenderPassEnabled(RenderPassHandle passHandle, bool isEnabled)
    {
        BaseT::setRenderPassEnabled(passHandle, isEnabled);
        m_creator.setRenderPassEnabled(passHandle, isEnabled);
    }

    void ActionCollectingScene::setRenderPassRenderOnce(RenderPassHandle passHandle, bool enable)
    {
        BaseT::setRenderPassRenderOnce(passHandle, enable);
        m_creator.setRenderPassRenderOnce(passHandle, enable);
    }

    void ActionCollectingScene::retriggerRenderPassRenderOnce(RenderPassHandle passHandle)
    {
        BaseT::retriggerRenderPassRenderOnce(passHandle);
        m_creator.retriggerRenderPassRenderOnce(passHandle);
    }

    void ActionCollectingScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        BaseT::addRenderGroupToRenderPass(passHandle, groupHandle, order);
        m_creator.addRenderGroupToRenderPass(passHandle, groupHandle, order);
    }

    void ActionCollectingScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        BaseT::removeRenderGroupFromRenderPass(passHandle, groupHandle);
        m_creator.removeRenderGroupFromRenderPass(passHandle, groupHandle);
    }

    BlitPassHandle ActionCollectingScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle handleActual = BaseT::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_creator.allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        BaseT::releaseBlitPass(passHandle);
        m_creator.releaseBlitPass(passHandle);
    }

    void ActionCollectingScene::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        BaseT::setBlitPassRenderOrder(passHandle, renderOrder);
        m_creator.setBlitPassRenderOrder(passHandle, renderOrder);
    }

    void ActionCollectingScene::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        BaseT::setBlitPassEnabled(passHandle, isEnabled);
        m_creator.setBlitPassEnabled(passHandle, isEnabled);
    }

    void ActionCollectingScene::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        BaseT::setBlitPassRegions(passHandle, sourceRegion, destinationRegion);
        m_creator.setBlitPassRegions(passHandle, sourceRegion, destinationRegion);
    }

    PickableObjectHandle ActionCollectingScene::allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle)
    {
        const PickableObjectHandle handleActual = BaseT::allocatePickableObject(geometryHandle, nodeHandle, id, pickableHandle);
        m_creator.allocatePickableObject(geometryHandle, nodeHandle, id, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releasePickableObject(PickableObjectHandle pickableHandle)
    {
        BaseT::releasePickableObject(pickableHandle);
        m_creator.releasePickableObject(pickableHandle);
    }

    void ActionCollectingScene::setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id)
    {
        BaseT::setPickableObjectId(pickableHandle, id);
        m_creator.setPickableObjectId(pickableHandle, id);
    }

    void ActionCollectingScene::setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle)
    {
        BaseT::setPickableObjectCamera(pickableHandle, cameraHandle);
        m_creator.setPickableObjectCamera(pickableHandle, cameraHandle);
    }

    void ActionCollectingScene::setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled)
    {
        BaseT::setPickableObjectEnabled(pickableHandle, isEnabled);
        m_creator.setPickableObjectEnabled(pickableHandle, isEnabled);
    }

    DataSlotHandle ActionCollectingScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle /*= DataSlotHandle::Invalid()*/)
    {
        const DataSlotHandle handleActual = BaseT::allocateDataSlot(dataSlot, handle);
        m_creator.allocateDataSlot(dataSlot, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseDataSlot(DataSlotHandle handle)
    {
        BaseT::releaseDataSlot(handle);
        m_creator.releaseDataSlot(handle);
    }

    void ActionCollectingScene::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        BaseT::setDataSlotTexture(handle, texture);
        m_creator.setDataSlotTexture(handle, texture);
    }

    TextureSamplerHandle ActionCollectingScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        const TextureSamplerHandle handleActual = BaseT::allocateTextureSampler(sampler, handle);
        m_creator.allocateTextureSampler(sampler, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        BaseT::releaseTextureSampler(handle);
        m_creator.releaseTextureSampler(handle);
    }

    // Render targets
    RenderTargetHandle ActionCollectingScene::allocateRenderTarget(RenderTargetHandle targetHandle)
    {
        const RenderTargetHandle handleActual = BaseT::allocateRenderTarget(targetHandle);
        m_creator.allocateRenderTarget(handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderTarget  (RenderTargetHandle targetHandle)
    {
        BaseT::releaseRenderTarget( targetHandle );
        m_creator.releaseRenderTarget(targetHandle);
    }

    RenderBufferHandle ActionCollectingScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle handleActual = BaseT::allocateRenderBuffer(renderBuffer, handle);
        m_creator.allocateRenderBuffer(renderBuffer, handleActual);
        return handleActual;
    }

    void ActionCollectingScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        BaseT::releaseRenderBuffer(handle);
        m_creator.releaseRenderBuffer(handle);
    }

    void ActionCollectingScene::setRenderBufferProperties(RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        BaseT::setRenderBufferProperties(handle, width, height, sampleCount);
        m_creator.setRenderBufferProperties(handle, width, height, sampleCount);
    }

    void ActionCollectingScene::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        BaseT::addRenderTargetRenderBuffer(targetHandle, bufferHandle);
        m_creator.addRenderTargetRenderBuffer(targetHandle, bufferHandle);
    }

    void ActionCollectingScene::setRenderPassClearColor(RenderPassHandle pass, const glm::vec4& clearColor)
    {
        BaseT::setRenderPassClearColor(pass, clearColor);
        m_creator.setRenderPassClearColor(pass, clearColor);
    }

    void ActionCollectingScene::setRenderPassClearFlag(RenderPassHandle pass, ClearFlags clearFlag)
    {
        BaseT::setRenderPassClearFlag(pass, clearFlag);
        m_creator.setRenderPassClearFlag(pass, clearFlag);
    }

    DataBufferHandle ActionCollectingScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle allocatedHandle = BaseT::allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, handle);
        m_creator.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, allocatedHandle);

        return allocatedHandle;
    }

    void ActionCollectingScene::releaseDataBuffer(DataBufferHandle handle)
    {
        BaseT::releaseDataBuffer(handle);
        m_creator.releaseDataBuffer(handle);
    }

    void ActionCollectingScene::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        BaseT::updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        m_creator.updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
    }

    UniformBufferHandle ActionCollectingScene::allocateUniformBuffer(uint32_t size, UniformBufferHandle handle)
    {
        const auto allocatedHandle = BaseT::allocateUniformBuffer(size, handle);
        m_creator.allocateUniformBuffer(size, allocatedHandle);
        return allocatedHandle;
    }

    void ActionCollectingScene::releaseUniformBuffer(UniformBufferHandle handle)
    {
        BaseT::releaseUniformBuffer(handle);
        m_creator.releaseUniformBuffer(handle);
    }

    void ActionCollectingScene::updateUniformBuffer(UniformBufferHandle uniformBufferHandle, uint32_t offset, uint32_t size, const std::byte* data)
    {
        BaseT::updateUniformBuffer(uniformBufferHandle, offset, size, data);
        m_creator.updateUniformBuffer(uniformBufferHandle, offset, size, data);
    }

    TextureBufferHandle ActionCollectingScene::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        const TextureBufferHandle allocatedHandle = BaseT::allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        m_creator.allocateTextureBuffer(textureFormat, mipMapDimensions, allocatedHandle);

        return allocatedHandle;
    }

    void ActionCollectingScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        BaseT::releaseTextureBuffer(handle);
        m_creator.releaseTextureBuffer(handle);
    }

    void ActionCollectingScene::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        BaseT::updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        const uint32_t dataSize = width * height * GetTexelSizeFromFormat(getTextureBuffer(handle).textureFormat);
        m_creator.updateTextureBuffer(handle, mipLevel, x, y, width, height, data, dataSize);
    }

    SceneReferenceHandle ActionCollectingScene::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        const auto actualHandle = BaseT::allocateSceneReference(sceneId, handle);
        m_creator.allocateSceneReference(sceneId, actualHandle);
        return actualHandle;
    }

    void ActionCollectingScene::releaseSceneReference(SceneReferenceHandle handle)
    {
        BaseT::releaseSceneReference(handle);
        m_creator.releaseSceneReference(handle);
    }

    void ActionCollectingScene::requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state)
    {
        BaseT::requestSceneReferenceState(handle, state);
        m_creator.requestSceneReferenceState(handle, state);
    }

    void ActionCollectingScene::requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable)
    {
        BaseT::requestSceneReferenceFlushNotifications(handle, enable);
        m_creator.requestSceneReferenceFlushNotifications(handle, enable);
    }

    void ActionCollectingScene::setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder)
    {
        BaseT::setSceneReferenceRenderOrder(handle, renderOrder);
        m_creator.setSceneReferenceRenderOrder(handle, renderOrder);
    }

    const SceneActionCollection& ActionCollectingScene::getSceneActionCollection() const
    {
        return m_collection;
    }

    SceneActionCollection& ActionCollectingScene::getSceneActionCollection()
    {
        return m_collection;
    }

    void ActionCollectingScene::linkData(SceneReferenceHandle providerScene, DataSlotId providerId, SceneReferenceHandle consumerScene, DataSlotId consumerId)
    {
        SceneReferenceAction action;
        action.type = SceneReferenceActionType::LinkData;
        action.providerScene = providerScene;
        action.providerId = providerId;
        action.consumerScene = consumerScene;
        action.consumerId = consumerId;
        m_sceneReferenceActions.push_back(std::move(action));
    }

    void ActionCollectingScene::unlinkData(SceneReferenceHandle consumerScene, DataSlotId consumerId)
    {
        SceneReferenceAction action;
        action.type = SceneReferenceActionType::UnlinkData;
        action.consumerScene = consumerScene;
        action.consumerId = consumerId;
        m_sceneReferenceActions.push_back(std::move(action));
    }

    const SceneReferenceActionVector& ActionCollectingScene::getSceneReferenceActions() const
    {
        return m_sceneReferenceActions;
    }

    void ActionCollectingScene::resetSceneReferenceActions()
    {
        m_sceneReferenceActions.clear();
    }
}
