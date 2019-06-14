//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ActionTestScene.h"

namespace ramses_internal
{
    ActionTestScene::ActionTestScene(const SceneInfo& sceneInfo)
        : m_scene(sceneInfo)
        , m_actionApplier(const_cast<Scene&>(m_scene))   // this const cast is needed to enforce usage of converter instead of m_scene
        , m_actionCollector()
    {
    }

    const String& ActionTestScene::getName() const
    {
        return m_scene.getName();
    }

    SceneId ActionTestScene::getSceneId() const
    {
        return m_scene.getSceneId();
    }

    UInt32 ActionTestScene::getRenderableCount() const
    {
        return m_scene.getRenderableCount();
    }

    RenderableHandle ActionTestScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle /*= RenderableHandle::Invalid()*/)
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

    Bool ActionTestScene::isRenderableAllocated(RenderableHandle renderableHandle) const
    {
        return m_scene.isRenderableAllocated(renderableHandle);
    }

    void ActionTestScene::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        m_actionCollector.setRenderableEffect(renderableHandle, effectHash);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        m_actionCollector.setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex)
    {
        m_actionCollector.setRenderableStartIndex(renderableHandle, startIndex);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount)
    {
        m_actionCollector.setRenderableIndexCount(renderableHandle, indexCount);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        m_actionCollector.setRenderableRenderState(renderableHandle, stateHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableVisibility(RenderableHandle renderableHandle, Bool visible)
    {
        m_actionCollector.setRenderableVisibility(renderableHandle, visible);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount)
    {
        m_actionCollector.setRenderableInstanceCount(renderableHandle, instanceCount);
        flushPendingSceneActions();
    }

    const Renderable& ActionTestScene::getRenderable(RenderableHandle renderableHandle) const
    {
        return m_scene.getRenderable(renderableHandle);
    }

    UInt32 ActionTestScene::getRenderStateCount() const
    {
        return m_scene.getRenderStateCount();
    }

    RenderStateHandle ActionTestScene::allocateRenderState(RenderStateHandle stateHandle /*= RenderStateHandle::Invalid()*/)
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

    Bool ActionTestScene::isRenderStateAllocated(RenderStateHandle stateHandle) const
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

    void ActionTestScene::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask)
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

    UInt32 ActionTestScene::getCameraCount() const
    {
        return m_scene.getCameraCount();
    }

    CameraHandle ActionTestScene::allocateCamera(ECameraProjectionType projType, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle /*= CameraHandle::Invalid()*/)
    {
        const CameraHandle actualHandle = m_actionCollector.allocateCamera(projType, nodeHandle, viewportDataInstance, handle);
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

    void ActionTestScene::setCameraFrustum(CameraHandle cameraHandle, const Frustum& frustum)
    {
        m_actionCollector.setCameraFrustum(cameraHandle, frustum);
        flushPendingSceneActions();
    }

    UInt32 ActionTestScene::getNodeCount() const
    {
        return m_scene.getNodeCount();
    }

    NodeHandle ActionTestScene::allocateNode(UInt32 childrenCount, NodeHandle handle /*= NodeHandle::Invalid()*/)
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

    Bool ActionTestScene::isNodeAllocated(NodeHandle nodeHandle) const
    {
        return m_scene.isNodeAllocated(nodeHandle);
    }

    UInt32 ActionTestScene::getTransformCount() const
    {
        return m_scene.getTransformCount();
    }

    TransformHandle ActionTestScene::allocateTransform(NodeHandle nodeHandle, TransformHandle handle /*= TransformHandle::Invalid()*/)
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

    Bool ActionTestScene::isTransformAllocated(TransformHandle transformHandle) const
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

    UInt32 ActionTestScene::getChildCount(NodeHandle parent) const
    {
        return m_scene.getChildCount(parent);
    }

    NodeHandle ActionTestScene::getChild(NodeHandle parent, UInt32 childNumber) const
    {
        return m_scene.getChild(parent, childNumber);
    }

    NodeHandle ActionTestScene::getTransformNode(TransformHandle handle) const
    {
        return m_scene.getTransformNode(handle);
    }

    const Vector3& ActionTestScene::getTranslation(TransformHandle handle) const
    {
        return m_scene.getTranslation(handle);
    }

    const Vector3& ActionTestScene::getRotation(TransformHandle handle) const
    {
        return m_scene.getRotation(handle);
    }

    const Vector3& ActionTestScene::getScaling(TransformHandle handle) const
    {
        return m_scene.getScaling(handle);
    }

    void ActionTestScene::setTranslation(TransformHandle handle, const Vector3& translation)
    {
        m_actionCollector.setTranslation(handle, translation);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRotation(TransformHandle handle, const Vector3& rotation)
    {
        m_actionCollector.setRotation(handle, rotation);
        flushPendingSceneActions();
    }

    void ActionTestScene::setScaling(TransformHandle handle, const Vector3& scaling)
    {
        m_actionCollector.setScaling(handle, scaling);
        flushPendingSceneActions();
    }

    UInt32 ActionTestScene::getDataLayoutCount() const
    {
        return m_scene.getDataLayoutCount();
    }

    DataLayoutHandle ActionTestScene::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        const DataLayoutHandle actualHandle = m_actionCollector.allocateDataLayout(dataFields, handle);
        flushPendingSceneActions();
        return actualHandle;
    }

    void ActionTestScene::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        m_actionCollector.releaseDataLayout(layoutHandle);
        flushPendingSceneActions();
    }

    Bool ActionTestScene::isDataLayoutAllocated(DataLayoutHandle layoutHandle) const
    {
        return m_scene.isDataLayoutAllocated(layoutHandle);
    }

    const DataLayout& ActionTestScene::getDataLayout(DataLayoutHandle layoutHandle) const
    {
        return m_scene.getDataLayout(layoutHandle);
    }

    UInt32 ActionTestScene::getDataInstanceCount() const
    {
        return m_scene.getDataInstanceCount();
    }

    DataInstanceHandle ActionTestScene::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle /*= DataInstanceHandle::Invalid()*/)
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

    Bool ActionTestScene::isDataInstanceAllocated(DataInstanceHandle containerHandle) const
    {
        return m_scene.isDataInstanceAllocated(containerHandle);
    }

    DataLayoutHandle ActionTestScene::getLayoutOfDataInstance(DataInstanceHandle containerHandle) const
    {
        return m_scene.getLayoutOfDataInstance(containerHandle);
    }

    const Float* ActionTestScene::getDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataFloatArray(containerHandle, field);
    }

    const Vector2* ActionTestScene::getDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector2fArray(containerHandle, field);
    }

    const Vector3* ActionTestScene::getDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector3fArray(containerHandle, field);
    }

    const Vector4* ActionTestScene::getDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector4fArray(containerHandle, field);
    }

    const Int32* ActionTestScene::getDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataIntegerArray(containerHandle, field);
    }

    const Matrix22f* ActionTestScene::getDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix22fArray(containerHandle, field);
    }

    const Matrix33f* ActionTestScene::getDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix33fArray(containerHandle, field);
    }

    const Matrix44f* ActionTestScene::getDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataMatrix44fArray(containerHandle, field);
    }

    const Vector2i* ActionTestScene::getDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector2iArray(containerHandle, field);
    }

    const Vector3i* ActionTestScene::getDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataVector3iArray(containerHandle, field);
    }

    const Vector4i* ActionTestScene::getDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
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

    Float ActionTestScene::getDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleFloat(containerHandle, field);
    }

    const Vector2& ActionTestScene::getDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector2f(containerHandle, field);
    }

    const Vector3& ActionTestScene::getDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector3f(containerHandle, field);
    }

    const Vector4& ActionTestScene::getDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector4f(containerHandle, field);
    }

    Int32 ActionTestScene::getDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleInteger(containerHandle, field);
    }

    const Matrix22f& ActionTestScene::getDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix22f(containerHandle, field);
    }

    const Matrix33f& ActionTestScene::getDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix33f(containerHandle, field);
    }

    const Matrix44f& ActionTestScene::getDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleMatrix44f(containerHandle, field);
    }

    const Vector2i& ActionTestScene::getDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector2i(containerHandle, field);
    }

    const Vector3i& ActionTestScene::getDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector3i(containerHandle, field);
    }

    const Vector4i& ActionTestScene::getDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_scene.getDataSingleVector4i(containerHandle, field);
    }

    void ActionTestScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data)
    {
        m_actionCollector.setDataFloatArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data)
    {
        m_actionCollector.setDataVector2fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data)
    {
        m_actionCollector.setDataVector3fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data)
    {
        m_actionCollector.setDataVector4fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data)
    {
        m_actionCollector.setDataMatrix22fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data)
    {
        m_actionCollector.setDataMatrix33fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data)
    {
        m_actionCollector.setDataMatrix44fArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        m_actionCollector.setDataIntegerArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data)
    {
        m_actionCollector.setDataVector2iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data)
    {
        m_actionCollector.setDataVector3iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data)
    {
        m_actionCollector.setDataVector4iArray(containerHandle, field, elementCount, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        m_actionCollector.setDataResource(containerHandle, field, hash, dataBuffer, instancingDivisor);
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

    void ActionTestScene::setDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field, Float data)
    {
        m_actionCollector.setDataSingleFloat(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2& data)
    {
        m_actionCollector.setDataSingleVector2f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3& data)
    {
        m_actionCollector.setDataSingleVector3f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4& data)
    {
        m_actionCollector.setDataSingleVector4f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field, Int32 data)
    {
        m_actionCollector.setDataSingleInteger(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2i& data)
    {
        m_actionCollector.setDataSingleVector2i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3i& data)
    {
        m_actionCollector.setDataSingleVector3i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4i& data)
    {
        m_actionCollector.setDataSingleVector4i(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix22f& data)
    {
        m_actionCollector.setDataSingleMatrix22f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix33f& data)
    {
        m_actionCollector.setDataSingleMatrix33f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    void ActionTestScene::setDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix44f& data)
    {
        m_actionCollector.setDataSingleMatrix44f(containerHandle, field, data);
        flushPendingSceneActions();
    }

    TextureSamplerHandle ActionTestScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle /*= TextureSamplerHandle::Invalid()*/)
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

    UInt32 ActionTestScene::getTextureSamplerCount() const
    {
        return m_scene.getTextureSamplerCount();
    }

    const TextureSampler& ActionTestScene::getTextureSampler(TextureSamplerHandle handle) const
    {
        return m_scene.getTextureSampler(handle);
    }

    Bool ActionTestScene::isTextureSamplerAllocated(TextureSamplerHandle samplerHandle) const
    {
        return m_scene.isTextureSamplerAllocated(samplerHandle);
    }

    AnimationSystemHandle ActionTestScene::addAnimationSystem(IAnimationSystem* animationSystem, AnimationSystemHandle animSystemHandle)
    {
        auto handle = m_actionCollector.addAnimationSystem(animationSystem, animSystemHandle);
        flushPendingSceneActions();
        return handle;
    }

    void ActionTestScene::removeAnimationSystem(AnimationSystemHandle animSystemHandle)
    {
        m_actionCollector.removeAnimationSystem(animSystemHandle);
        flushPendingSceneActions();
    }

    const IAnimationSystem* ActionTestScene::getAnimationSystem(AnimationSystemHandle animSystemHandle) const
    {
        return m_scene.getAnimationSystem(animSystemHandle);
    }

    IAnimationSystem* ActionTestScene::getAnimationSystem(AnimationSystemHandle animSystemHandle)
    {
        return const_cast<Scene&>(m_scene).getAnimationSystem(animSystemHandle);
    }

    Bool ActionTestScene::isAnimationSystemAllocated(AnimationSystemHandle animSystemHandle) const
    {
        return m_scene.isAnimationSystemAllocated(animSystemHandle);
    }

    UInt32 ActionTestScene::getAnimationSystemCount() const
    {
        return m_scene.getAnimationSystemCount();
    }

    SceneSizeInformation ActionTestScene::getSceneSizeInformation() const
    {
        return m_scene.getSceneSizeInformation();
    }

    RenderGroupHandle ActionTestScene::allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle)
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

    Bool ActionTestScene::isRenderGroupAllocated(RenderGroupHandle groupHandle) const
    {
        return m_scene.isRenderGroupAllocated(groupHandle);
    }

    UInt32 ActionTestScene::getRenderGroupCount() const
    {
        return m_scene.getRenderGroupCount();
    }

    void ActionTestScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order)
    {
        m_actionCollector.addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        flushPendingSceneActions();
    }

    void ActionTestScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        m_actionCollector.removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order)
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

    RenderPassHandle ActionTestScene::allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle handle /*= RenderPassHandle::Invalid()*/)
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

    Bool ActionTestScene::isRenderPassAllocated(RenderPassHandle pass) const
    {
        return m_scene.isRenderPassAllocated(pass);
    }

    UInt32 ActionTestScene::getRenderPassCount() const
    {
        return m_scene.getRenderPassCount();
    }

    void ActionTestScene::setRenderPassCamera(RenderPassHandle pass, CameraHandle camera)
    {
        m_actionCollector.setRenderPassCamera(pass, camera);
        flushPendingSceneActions();
    }

    RenderTargetHandle ActionTestScene::allocateRenderTarget(RenderTargetHandle targetHandle /*= RenderTargetHandle::Invalid()*/)
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

    UInt32 ActionTestScene::getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const
    {
        return m_scene.getRenderTargetRenderBufferCount(targetHandle);
    }

    RenderBufferHandle ActionTestScene::getRenderTargetRenderBuffer(RenderTargetHandle targetHandle, UInt32 bufferIndex) const
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

    Bool ActionTestScene::isRenderBufferAllocated(RenderBufferHandle handle) const
    {
        return m_scene.isRenderBufferAllocated(handle);
    }

    UInt32 ActionTestScene::getRenderBufferCount() const
    {
        return m_scene.getRenderBufferCount();
    }

    const RenderBuffer& ActionTestScene::getRenderBuffer(RenderBufferHandle handle) const
    {
        return m_scene.getRenderBuffer(handle);
    }

    StreamTextureHandle ActionTestScene::allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle)
    {
        const StreamTextureHandle handle = m_actionCollector.allocateStreamTexture(streamSource, fallbackTextureHash, streamTextureHandle);
        flushPendingSceneActions();
        return handle;
    }

    void ActionTestScene::releaseStreamTexture(StreamTextureHandle streamTextureHandle)
    {
        m_actionCollector.releaseStreamTexture(streamTextureHandle);
        flushPendingSceneActions();
    }

    Bool ActionTestScene::isStreamTextureAllocated(StreamTextureHandle streamTextureHandle) const
    {
        return m_scene.isStreamTextureAllocated(streamTextureHandle);
    }

    UInt32 ActionTestScene::getStreamTextureCount() const
    {
        return m_scene.getStreamTextureCount();
    }

    void ActionTestScene::setForceFallbackImage(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage)
    {
        m_actionCollector.setForceFallbackImage(streamTextureHandle, forceFallbackImage);
        flushPendingSceneActions();
    }

    const StreamTexture& ActionTestScene::getStreamTexture(StreamTextureHandle streamTextureHandle) const
    {
        return m_scene.getStreamTexture(streamTextureHandle);
    }

    DataBufferHandle ActionTestScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
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

    UInt32 ActionTestScene::getDataBufferCount() const
    {
        return m_scene.getDataBufferCount();
    }

    void ActionTestScene::updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data)
    {
        m_actionCollector.updateDataBuffer(handle, offsetInBytes, dataSizeInBytes, data);
        flushPendingSceneActions();
    }

    Bool ActionTestScene::isDataBufferAllocated(DataBufferHandle handle) const
    {
        return m_scene.isDataBufferAllocated(handle);
    }

    const GeometryDataBuffer& ActionTestScene::getDataBuffer(DataBufferHandle handle) const
    {
        return m_scene.getDataBuffer(handle);
    }

    TextureBufferHandle ActionTestScene::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
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

    UInt32 ActionTestScene::getTextureBufferCount() const
    {
        return m_scene.getTextureBufferCount();
    }

    void ActionTestScene::updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data)
    {
        m_actionCollector.updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        flushPendingSceneActions();
    }

    const TextureBuffer& ActionTestScene::getTextureBuffer(TextureBufferHandle handle) const
    {
        return m_scene.getTextureBuffer(handle);
    }

    Bool ActionTestScene::isTextureBufferAllocated(TextureBufferHandle handle) const
    {
        return m_scene.isTextureBufferAllocated(handle);
    }

    DataSlotHandle ActionTestScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle /*= DataSlotHandle::Invalid()*/)
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

    Bool ActionTestScene::isDataSlotAllocated(DataSlotHandle handle) const
    {
        return m_scene.isDataSlotAllocated(handle);
    }

    UInt32 ActionTestScene::getDataSlotCount() const
    {
        return m_scene.getDataSlotCount();
    }

    const DataSlot& ActionTestScene::getDataSlot(DataSlotHandle handle) const
    {
        return m_scene.getDataSlot(handle);
    }

    void ActionTestScene::setRenderPassClearFlag(RenderPassHandle handle, UInt32 clearFlag)
    {
        m_actionCollector.setRenderPassClearFlag(handle, clearFlag);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassClearColor(RenderPassHandle handle, const Vector4& clearColor)
    {
        m_actionCollector.setRenderPassClearColor(handle, clearColor);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderTarget(RenderPassHandle pass, RenderTargetHandle targetHandle)
    {
        m_actionCollector.setRenderPassRenderTarget(pass, targetHandle);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderOrder(RenderPassHandle pass, Int32 renderOrder)
    {
        m_actionCollector.setRenderPassRenderOrder(pass, renderOrder);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassEnabled(RenderPassHandle pass, Bool isEnabled)
    {
        m_actionCollector.setRenderPassEnabled(pass, isEnabled);
        flushPendingSceneActions();
    }

    void ActionTestScene::setRenderPassRenderOnce(RenderPassHandle pass, Bool enable)
    {
        m_actionCollector.setRenderPassRenderOnce(pass, enable);
        flushPendingSceneActions();
    }

    void ActionTestScene::retriggerRenderPassRenderOnce(RenderPassHandle pass)
    {
        m_actionCollector.retriggerRenderPassRenderOnce(pass);
        flushPendingSceneActions();
    }

    void ActionTestScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order)
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

    BlitPassHandle ActionTestScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
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

    Bool ActionTestScene::isBlitPassAllocated(BlitPassHandle passHandle) const
    {
        return m_scene.isBlitPassAllocated(passHandle);
    }

    ramses_internal::UInt32 ActionTestScene::getBlitPassCount() const
    {
        return m_scene.getBlitPassCount();
    }

    void ActionTestScene::setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder)
    {
        m_actionCollector.setBlitPassRenderOrder(passHandle, renderOrder);
        flushPendingSceneActions();
    }

    void ActionTestScene::setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled)
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

    Bool ActionTestScene::isRenderTargetAllocated(RenderTargetHandle targetHandle) const
    {
        return m_scene.isRenderTargetAllocated(targetHandle);
    }

    UInt32 ActionTestScene::getRenderTargetCount() const
    {
        return m_scene.getRenderTargetCount();
    }

    Bool ActionTestScene::isCameraAllocated(CameraHandle handle) const
    {
        return m_scene.isCameraAllocated(handle);
    }

    void ActionTestScene::flushPendingSceneActions()
    {
        SceneActionCollection& actionCollector = m_actionCollector.getSceneActionCollection();
        m_actionApplier.applyActionsOnScene(actionCollector);
        m_actionCollector.getSceneActionCollection().clear();
    }

    void ActionTestScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        m_actionCollector.preallocateSceneSize(sizeInfo);
        flushPendingSceneActions();
    }
}
