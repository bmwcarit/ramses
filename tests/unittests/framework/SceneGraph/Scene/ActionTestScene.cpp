//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ActionTestScene.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"

namespace ramses::internal
{
    ActionTestScene::ActionTestScene(const SceneInfo& sceneInfo)
        : m_scene(sceneInfo)
    {
    }

    const std::string& ActionTestScene::getName() const
    {
        return m_scene.getName();
    }

    SceneId ActionTestScene::getSceneId() const
    {
        return m_scene.getSceneId();
    }

    void ActionTestScene::setEffectTimeSync(FlushTime::Clock::time_point /*t*/)
    {
        // not set by a scene action
    }

    FlushTime::Clock::time_point ActionTestScene::getEffectTimeSync() const
    {
        return m_scene.getEffectTimeSync();
    }

    uint32_t ActionTestScene::getRenderableCount() const
    {
        return m_scene.getRenderableCount();
    }

    RenderableHandle ActionTestScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        const RenderableHandle actualHandle = m_actionCollector.allocateRenderable(nodeHandle, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        m_actionCollector.releaseRenderable(renderableHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isRenderableAllocated(RenderableHandle renderableHandle) const
    {
        return m_scene.isRenderableAllocated(renderableHandle);
    }

    void ActionTestScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        m_actionCollector.setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex)
    {
        m_actionCollector.setRenderableStartIndex(renderableHandle, startIndex);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount)
    {
        m_actionCollector.setRenderableIndexCount(renderableHandle, indexCount);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        m_actionCollector.setRenderableRenderState(renderableHandle, stateHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible)
    {
        m_actionCollector.setRenderableVisibility(renderableHandle, visible);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount)
    {
        m_actionCollector.setRenderableInstanceCount(renderableHandle, instanceCount);
        flushPendingSceneActions();
    }


    void ActionTestScene::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        m_actionCollector.setRenderableStartVertex(renderableHandle, startVertex);
        flushPendingSceneActions();
    }

    const Renderable& ActionTestScene::getRenderable(RenderableHandle renderableHandle) const
    {
        return m_scene.getRenderable(renderableHandle);
    }

    uint32_t ActionTestScene::getRenderStateCount() const
    {
        return m_scene.getRenderStateCount();
    }

    RenderStateHandle ActionTestScene::allocateRenderState(RenderStateHandle stateHandle)
    {
        const RenderStateHandle actualHandle = m_actionCollector.allocateRenderState(stateHandle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderState(RenderStateHandle stateHandle)
    {
        m_actionCollector.releaseRenderState(stateHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isRenderStateAllocated(RenderStateHandle stateHandle) const
    {
        return m_scene.isRenderStateAllocated(stateHandle);
    }

    void ActionTestScene::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        m_actionCollector.setRenderStateBlendFactors(stateHandle, srcColor, destColor, srcAlpha, destAlpha);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        m_actionCollector.setRenderStateBlendOperations(stateHandle, operationColor, operationAlpha);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color)
    {
        m_actionCollector.setRenderStateBlendColor(stateHandle, color);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        m_actionCollector.setRenderStateCullMode(stateHandle, cullMode);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        m_actionCollector.setRenderStateDrawMode(stateHandle, drawMode);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        m_actionCollector.setRenderStateDepthFunc(stateHandle, func);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        m_actionCollector.setRenderStateDepthWrite(stateHandle, flag);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        m_actionCollector.setRenderStateScissorTest(stateHandle, flag, region);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        m_actionCollector.setRenderStateStencilFunc(stateHandle, func, ref, mask);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        m_actionCollector.setRenderStateStencilOps(stateHandle, sfail, dpfail, dppass);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        m_actionCollector.setRenderStateColorWriteMask(stateHandle, colorMask);
        flushPendingSceneActions();
    }

    const RenderState& ActionTestScene::getRenderState(RenderStateHandle stateHandle) const
    {
        return m_scene.getRenderState(stateHandle);
    }

    uint32_t ActionTestScene::getCameraCount() const
    {
        return m_scene.getCameraCount();
    }

    CameraHandle ActionTestScene::allocateCamera(ECameraProjectionType projType, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        const CameraHandle actualHandle = m_actionCollector.allocateCamera(projType, nodeHandle, dataInstance, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseCamera(CameraHandle cameraHandle)
    {
        m_actionCollector.releaseCamera(cameraHandle);
        flushPendingSceneActions();
    }

    const Camera& ActionTestScene::getCamera(CameraHandle cameraHandle) const
    {
        return m_scene.getCamera(cameraHandle);
    }

    uint32_t ActionTestScene::getNodeCount() const
    {
        return m_scene.getNodeCount();
    }

    NodeHandle ActionTestScene::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        const NodeHandle actualHandle = m_actionCollector.allocateNode(childrenCount, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseNode(NodeHandle nodeHandle)
    {
        m_actionCollector.releaseNode(nodeHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isNodeAllocated(NodeHandle nodeHandle) const
    {
        return m_scene.isNodeAllocated(nodeHandle);
    }

    uint32_t ActionTestScene::getTransformCount() const
    {
        return m_scene.getTransformCount();
    }

    TransformHandle ActionTestScene::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        const TransformHandle actualHandle = m_actionCollector.allocateTransform(nodeHandle, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseTransform(TransformHandle transform)
    {
        m_actionCollector.releaseTransform(transform);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isTransformAllocated(TransformHandle transformHandle) const
    {
        return m_scene.isTransformAllocated(transformHandle);
    }

    NodeHandle ActionTestScene::getParent(NodeHandle nodeHandle) const
    {
        return m_scene.getParent(nodeHandle);
    }

    void ActionTestScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        m_actionCollector.addChildToNode(parent, child);
        flushPendingSceneActions();
    }

    void ActionTestScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        m_actionCollector.removeChildFromNode(parent, child);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getChildCount(NodeHandle parent) const
    {
        return m_scene.getChildCount(parent);
    }

    NodeHandle ActionTestScene::getChild(NodeHandle parent, uint32_t childNumber) const
    {
        return m_scene.getChild(parent, childNumber);
    }

    NodeHandle ActionTestScene::getTransformNode(TransformHandle handle) const
    {
        return m_scene.getTransformNode(handle);
    }

    const glm::vec3& ActionTestScene::getTranslation(TransformHandle handle) const
    {
        return m_scene.getTranslation(handle);
    }

    const glm::vec4& ActionTestScene::getRotation(TransformHandle handle) const
    {
        return m_scene.getRotation(handle);
    }

    ERotationType ActionTestScene::getRotationType(TransformHandle handle) const
    {
        return m_scene.getRotationType(handle);
    }

    const glm::vec3& ActionTestScene::getScaling(TransformHandle handle) const
    {
        return m_scene.getScaling(handle);
    }

    void ActionTestScene::setTranslation(TransformHandle handle, const glm::vec3& translation)
    {
        m_actionCollector.setTranslation(handle, translation);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRotation(TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType)
    {
        m_actionCollector.setRotation(handle, rotation, rotationType);
        flushPendingSceneActions();
    }

    void ActionTestScene::setScaling(TransformHandle handle, const glm::vec3& scaling)
    {
        m_actionCollector.setScaling(handle, scaling);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getDataLayoutCount() const
    {
        return m_scene.getDataLayoutCount();
    }

    DataLayoutHandle ActionTestScene::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        const DataLayoutHandle actualHandle = m_actionCollector.allocateDataLayout(dataFields, effectHash, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        m_actionCollector.releaseDataLayout(layoutHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isDataLayoutAllocated(DataLayoutHandle layoutHandle) const
    {
        return m_scene.isDataLayoutAllocated(layoutHandle);
    }

    const DataLayout& ActionTestScene::getDataLayout(DataLayoutHandle layoutHandle) const
    {
        return m_scene.getDataLayout(layoutHandle);
    }

    uint32_t ActionTestScene::getDataInstanceCount() const
    {
        return m_scene.getDataInstanceCount();
    }

    DataInstanceHandle ActionTestScene::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle)
    {
        const DataInstanceHandle actualHandle = m_actionCollector.allocateDataInstance(finishedLayoutHandle, instanceHandle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseDataInstance(DataInstanceHandle containerHandle)
    {
        m_actionCollector.releaseDataInstance(containerHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isDataInstanceAllocated(DataInstanceHandle containerHandle) const
    {
        return m_scene.isDataInstanceAllocated(containerHandle);
    }

    DataLayoutHandle ActionTestScene::getLayoutOfDataInstance(DataInstanceHandle containerHandle) const
    {
        return m_scene.getLayoutOfDataInstance(containerHandle);
    }

    const float* ActionTestScene::getDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataFloatArray(containerHandle, field);
    }

    const glm::vec2* ActionTestScene::getDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector2fArray(containerHandle, field);
    }

    const glm::vec3* ActionTestScene::getDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector3fArray(containerHandle, field);
    }

    const glm::vec4* ActionTestScene::getDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector4fArray(containerHandle, field);
    }

    const bool* ActionTestScene::getDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataBooleanArray(containerHandle, field);
    }

    const int32_t* ActionTestScene::getDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataIntegerArray(containerHandle, field);
    }

    const glm::mat2* ActionTestScene::getDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix22fArray(containerHandle, field);
    }

    const glm::mat3* ActionTestScene::getDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix33fArray(containerHandle, field);
    }

    const glm::mat4* ActionTestScene::getDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix44fArray(containerHandle, field);
    }

    const glm::ivec2* ActionTestScene::getDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector2iArray(containerHandle, field);
    }

    const glm::ivec3* ActionTestScene::getDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector3iArray(containerHandle, field);
    }

    const glm::ivec4* ActionTestScene::getDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector4iArray(containerHandle, field);
    }

    const ResourceField& ActionTestScene::getDataResource(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataResource(containerHandle, field);
    }

    TextureSamplerHandle ActionTestScene::getDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataTextureSamplerHandle(containerHandle, field);
    }

    DataInstanceHandle ActionTestScene::getDataReference(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataReference(containerHandle, field);
    }

    float ActionTestScene::getDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleFloat(containerHandle, field);
    }

    const glm::vec2& ActionTestScene::getDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector2f(containerHandle, field);
    }

    const glm::vec3& ActionTestScene::getDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector3f(containerHandle, field);
    }

    const glm::vec4& ActionTestScene::getDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector4f(containerHandle, field);
    }

    bool ActionTestScene::getDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleBoolean(containerHandle, field);
    }

    int32_t ActionTestScene::getDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleInteger(containerHandle, field);
    }

    const glm::mat2& ActionTestScene::getDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix22f(containerHandle, field);
    }

    const glm::mat3& ActionTestScene::getDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix33f(containerHandle, field);
    }

    const glm::mat4& ActionTestScene::getDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix44f(containerHandle, field);
    }

    const glm::ivec2& ActionTestScene::getDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector2i(containerHandle, field);
    }

    const glm::ivec3& ActionTestScene::getDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector3i(containerHandle, field);
    }

    const glm::ivec4& ActionTestScene::getDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector4i(containerHandle, field);
    }

    void ActionTestScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data)
    {
        m_actionCollector.setDataFloatArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data)
    {
        m_actionCollector.setDataVector2fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data)
    {
        m_actionCollector.setDataVector3fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data)
    {
        m_actionCollector.setDataVector4fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data)
    {
        m_actionCollector.setDataMatrix22fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data)
    {
        m_actionCollector.setDataMatrix33fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data)
    {
        m_actionCollector.setDataMatrix44fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data)
    {
        m_actionCollector.setDataBooleanArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data)
    {
        m_actionCollector.setDataIntegerArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data)
    {
        m_actionCollector.setDataVector2iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data)
    {
        m_actionCollector.setDataVector3iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data)
    {
        m_actionCollector.setDataVector4iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        m_actionCollector.setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor, offsetWithinElementInBytes, stride);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        m_actionCollector.setDataTextureSamplerHandle(containerHandle, field, samplerHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        m_actionCollector.setDataReference(containerHandle, field, dataRef);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field, float data)
    {
        m_actionCollector.setDataSingleFloat(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec2& data)
    {
        m_actionCollector.setDataSingleVector2f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec3& data)
    {
        m_actionCollector.setDataSingleVector3f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec4& data)
    {
        m_actionCollector.setDataSingleVector4f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field, bool data)
    {
        m_actionCollector.setDataSingleBoolean(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field, int32_t data)
    {
        m_actionCollector.setDataSingleInteger(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec2& data)
    {
        m_actionCollector.setDataSingleVector2i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec3& data)
    {
        m_actionCollector.setDataSingleVector3i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec4& data)
    {
        m_actionCollector.setDataSingleVector4i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat2& data)
    {
        m_actionCollector.setDataSingleMatrix22f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat3& data)
    {
        m_actionCollector.setDataSingleMatrix33f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat4& data)
    {
        m_actionCollector.setDataSingleMatrix44f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    TextureSamplerHandle ActionTestScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        const TextureSamplerHandle actualHandle = m_actionCollector.allocateTextureSampler(sampler, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        m_actionCollector.releaseTextureSampler(handle);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getTextureSamplerCount() const
    {
        return m_scene.getTextureSamplerCount();
    }

    const TextureSampler& ActionTestScene::getTextureSampler(TextureSamplerHandle handle) const
    {
        return m_scene.getTextureSampler(handle);
    }

    bool ActionTestScene::isTextureSamplerAllocated(TextureSamplerHandle samplerHandle) const
    {
        return m_scene.isTextureSamplerAllocated(samplerHandle);
    }

    SceneSizeInformation ActionTestScene::getSceneSizeInformation() const
    {
        return m_scene.getSceneSizeInformation();
    }

    RenderGroupHandle ActionTestScene::allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle)
    {
        const RenderGroupHandle actualHandle = m_actionCollector.allocateRenderGroup(renderableCount, nestedGroupCount, groupHandle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        m_actionCollector.releaseRenderGroup(groupHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isRenderGroupAllocated(RenderGroupHandle groupHandle) const
    {
        return m_scene.isRenderGroupAllocated(groupHandle);
    }

    uint32_t ActionTestScene::getRenderGroupCount() const
    {
        return m_scene.getRenderGroupCount();
    }

    void ActionTestScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        m_actionCollector.addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        flushPendingSceneActions();
    }

    void ActionTestScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        m_actionCollector.removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        m_actionCollector.addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
        flushPendingSceneActions();
    }

    void ActionTestScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        m_actionCollector.removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
        flushPendingSceneActions();
    }

    const RenderGroup& ActionTestScene::getRenderGroup(RenderGroupHandle groupHandle) const
    {
        return m_scene.getRenderGroup(groupHandle);
    }

    RenderPassHandle ActionTestScene::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle)
    {
        const RenderPassHandle actualHandle = m_actionCollector.allocateRenderPass(renderGroupCount, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderPass(RenderPassHandle handle)
    {
        m_actionCollector.releaseRenderPass(handle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isRenderPassAllocated(RenderPassHandle pass) const
    {
        return m_scene.isRenderPassAllocated(pass);
    }

    uint32_t ActionTestScene::getRenderPassCount() const
    {
        return m_scene.getRenderPassCount();
    }

    void ActionTestScene::setRenderPassCamera(RenderPassHandle pass, CameraHandle camera)
    {
        m_actionCollector.setRenderPassCamera(pass, camera);
        flushPendingSceneActions();
    }

    RenderTargetHandle ActionTestScene::allocateRenderTarget(RenderTargetHandle targetHandle)
    {
        const RenderTargetHandle actualHandle = m_actionCollector.allocateRenderTarget(targetHandle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderTarget(RenderTargetHandle targetHandle)
    {
        m_actionCollector.releaseRenderTarget(targetHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        m_actionCollector.addRenderTargetRenderBuffer(targetHandle, bufferHandle);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const
    {
        return m_scene.getRenderTargetRenderBufferCount(targetHandle);
    }

    RenderBufferHandle ActionTestScene::getRenderTargetRenderBuffer(RenderTargetHandle targetHandle, uint32_t bufferIndex) const
    {
        return m_scene.getRenderTargetRenderBuffer(targetHandle, bufferIndex);
    }

    RenderBufferHandle ActionTestScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        const RenderBufferHandle actualHandle = m_actionCollector.allocateRenderBuffer(renderBuffer, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        m_actionCollector.releaseRenderBuffer(handle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isRenderBufferAllocated(RenderBufferHandle handle) const
    {
        return m_scene.isRenderBufferAllocated(handle);
    }

    uint32_t ActionTestScene::getRenderBufferCount() const
    {
        return m_scene.getRenderBufferCount();
    }

    const RenderBuffer& ActionTestScene::getRenderBuffer(RenderBufferHandle handle) const
    {
        return m_scene.getRenderBuffer(handle);
    }

    DataBufferHandle ActionTestScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        const DataBufferHandle allocatedHandle = m_actionCollector.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, handle);
        flushPendingSceneActions();
        return allocatedHandle;
    }

    void ActionTestScene::releaseDataBuffer(DataBufferHandle handle)
    {
        m_actionCollector.releaseDataBuffer(handle);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getDataBufferCount() const
    {
        return m_scene.getDataBufferCount();
    }

    void ActionTestScene::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        m_actionCollector.updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isDataBufferAllocated(DataBufferHandle handle) const
    {
        return m_scene.isDataBufferAllocated(handle);
    }

    const GeometryDataBuffer& ActionTestScene::getDataBuffer(DataBufferHandle handle) const
    {
        return m_scene.getDataBuffer(handle);
    }

    TextureBufferHandle ActionTestScene::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        const TextureBufferHandle actualHandle = m_actionCollector.allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        m_actionCollector.releaseTextureBuffer(handle);
        flushPendingSceneActions();
    }

    uint32_t ActionTestScene::getTextureBufferCount() const
    {
        return m_scene.getTextureBufferCount();
    }

    void ActionTestScene::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        m_actionCollector.updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        flushPendingSceneActions();
    }

    const TextureBuffer& ActionTestScene::getTextureBuffer(TextureBufferHandle handle) const
    {
        return m_scene.getTextureBuffer(handle);
    }

    bool ActionTestScene::isTextureBufferAllocated(TextureBufferHandle handle) const
    {
        return m_scene.isTextureBufferAllocated(handle);
    }

    DataSlotHandle ActionTestScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        const DataSlotHandle actualHandle = m_actionCollector.allocateDataSlot(dataSlot, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        m_actionCollector.setDataSlotTexture(handle, texture);
        flushPendingSceneActions();
    }

    void ActionTestScene::releaseDataSlot(DataSlotHandle handle)
    {
        m_actionCollector.releaseDataSlot(handle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isDataSlotAllocated(DataSlotHandle handle) const
    {
        return m_scene.isDataSlotAllocated(handle);
    }

    uint32_t ActionTestScene::getDataSlotCount() const
    {
        return m_scene.getDataSlotCount();
    }

    const DataSlot& ActionTestScene::getDataSlot(DataSlotHandle handle) const
    {
        return m_scene.getDataSlot(handle);
    }

    SceneReferenceHandle ActionTestScene::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        const auto actualHandle = m_actionCollector.allocateSceneReference(sceneId, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseSceneReference(SceneReferenceHandle handle)
    {
        m_actionCollector.releaseSceneReference(handle);
        flushPendingSceneActions();
    }

    void ActionTestScene::requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state)
    {
        m_actionCollector.requestSceneReferenceState(handle, state);
        flushPendingSceneActions();
    }

    void ActionTestScene::requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable)
    {
        m_actionCollector.requestSceneReferenceFlushNotifications(handle, enable);
        flushPendingSceneActions();
    }

    void ActionTestScene::setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder)
    {
        m_actionCollector.setSceneReferenceRenderOrder(handle, renderOrder);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isSceneReferenceAllocated(SceneReferenceHandle handle) const
    {
        return m_scene.isSceneReferenceAllocated(handle);
    }

    uint32_t ActionTestScene::getSceneReferenceCount() const
    {
        return m_scene.getSceneReferenceCount();
    }

    const SceneReference& ActionTestScene::getSceneReference(SceneReferenceHandle handle) const
    {
        return m_scene.getSceneReference(handle);
    }

    void ActionTestScene::setRenderPassClearFlag(RenderPassHandle handle, ClearFlags clearFlag)
    {
        m_actionCollector.setRenderPassClearFlag(handle, clearFlag);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassClearColor(RenderPassHandle handle, const glm::vec4& clearColor)
    {
        m_actionCollector.setRenderPassClearColor(handle, clearColor);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderTarget(RenderPassHandle pass, RenderTargetHandle targetHandle)
    {
        m_actionCollector.setRenderPassRenderTarget(pass, targetHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderOrder(RenderPassHandle pass, int32_t renderOrder)
    {
        m_actionCollector.setRenderPassRenderOrder(pass, renderOrder);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassEnabled(RenderPassHandle pass, bool isEnabled)
    {
        m_actionCollector.setRenderPassEnabled(pass, isEnabled);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderOnce(RenderPassHandle pass, bool enable)
    {
        m_actionCollector.setRenderPassRenderOnce(pass, enable);
        flushPendingSceneActions();
    }

    void ActionTestScene::retriggerRenderPassRenderOnce(RenderPassHandle pass)
    {
        m_actionCollector.retriggerRenderPassRenderOnce(pass);
        flushPendingSceneActions();
    }

    void ActionTestScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        m_actionCollector.addRenderGroupToRenderPass(passHandle, groupHandle, order);
        flushPendingSceneActions();
    }

    void ActionTestScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        m_actionCollector.removeRenderGroupFromRenderPass(passHandle, groupHandle);
        flushPendingSceneActions();
    }

    const RenderPass& ActionTestScene::getRenderPass(RenderPassHandle passHandle) const
    {
        return m_scene.getRenderPass(passHandle);
    }

    PickableObjectHandle ActionTestScene::allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle)
    {
        const PickableObjectHandle resultHandle = m_actionCollector.allocatePickableObject(geometryHandle, nodeHandle, id, pickableHandle);
        flushPendingSceneActions();
        return resultHandle;
    }

    void ActionTestScene::releasePickableObject(PickableObjectHandle pickableHandle)
    {
        m_actionCollector.releasePickableObject(pickableHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isPickableObjectAllocated(PickableObjectHandle pickableHandle) const
    {
        return m_scene.isPickableObjectAllocated(pickableHandle);
    }

    uint32_t ActionTestScene::getPickableObjectCount() const
    {
        return m_scene.getPickableObjectCount();
    }

    void ActionTestScene::setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id)
    {
        m_actionCollector.setPickableObjectId(pickableHandle, id);
        flushPendingSceneActions();
    }

    void ActionTestScene::setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle)
    {
        m_actionCollector.setPickableObjectCamera(pickableHandle, cameraHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled)
    {
        m_actionCollector.setPickableObjectEnabled(pickableHandle, isEnabled);
        flushPendingSceneActions();
    }

    const PickableObject& ActionTestScene::getPickableObject(PickableObjectHandle pickableHandle) const
    {
        return m_scene.getPickableObject(pickableHandle);
    }

    BlitPassHandle ActionTestScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        const BlitPassHandle resultHandle = m_actionCollector.allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        flushPendingSceneActions();

        return resultHandle;
    }

    void ActionTestScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        m_actionCollector.releaseBlitPass(passHandle);
        flushPendingSceneActions();
    }

    bool ActionTestScene::isBlitPassAllocated(BlitPassHandle passHandle) const
    {
        return m_scene.isBlitPassAllocated(passHandle);
    }

    uint32_t ActionTestScene::getBlitPassCount() const
    {
        return m_scene.getBlitPassCount();
    }

    void ActionTestScene::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        m_actionCollector.setBlitPassRenderOrder(passHandle, renderOrder);
        flushPendingSceneActions();
    }

    void ActionTestScene::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        m_actionCollector.setBlitPassEnabled(passHandle, isEnabled);
        flushPendingSceneActions();
    }

    void ActionTestScene::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        m_actionCollector.setBlitPassRegions(passHandle, sourceRegion, destinationRegion);
        flushPendingSceneActions();
    }

    const BlitPass& ActionTestScene::getBlitPass(BlitPassHandle passHandle) const
    {
        return m_scene.getBlitPass(passHandle);
    }

    bool ActionTestScene::isRenderTargetAllocated(RenderTargetHandle targetHandle) const
    {
        return m_scene.isRenderTargetAllocated(targetHandle);
    }

    uint32_t ActionTestScene::getRenderTargetCount() const
    {
        return m_scene.getRenderTargetCount();
    }

    bool ActionTestScene::isCameraAllocated(CameraHandle handle) const
    {
        return m_scene.isCameraAllocated(handle);
    }

    void ActionTestScene::flushPendingSceneActions()
    {
        SceneActionCollection& actionCollection = m_actionCollector.getSceneActionCollection();
        SceneActionApplier::ApplyActionsOnScene(const_cast<Scene&>(m_scene), actionCollection);
        m_actionCollector.getSceneActionCollection().clear();
    }

    void ActionTestScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        m_actionCollector.preallocateSceneSize(sizeInfo);
        flushPendingSceneActions();
    }
}
