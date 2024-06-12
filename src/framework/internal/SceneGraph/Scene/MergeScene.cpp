//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "MergeScene.h"
#include "internal/Core/Common/TypedMemoryHandle.h"
#include "internal/SceneGraph/Scene/DataLayoutCachedScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include <limits>

namespace ramses::internal
{
    template<>
    [[nodiscard]] inline TypedMemoryHandle<DataLayoutHandleTag> MergeScene::getMappedHandle(TypedMemoryHandle<DataLayoutHandleTag> handle) const;

    MergeScene::MergeScene(IScene& originalScene, SceneMergeHandleMapping& mapping)
        : m_originalScene{ originalScene }
        , m_mapping{ mapping }
    {
        const auto originalSizeInfo = m_originalScene.getSceneSizeInformation();

        m_offsetRenderableHandle = RenderableHandle{originalSizeInfo.renderableCount};
        m_offsetRenderStateHandle = RenderStateHandle(originalSizeInfo.renderStateCount);
        m_offsetCameraHandle = CameraHandle(originalSizeInfo.cameraCount);
        m_offsetNodeHandle = NodeHandle(originalSizeInfo.nodeCount);
        m_offsetTransformHandle = TransformHandle(originalSizeInfo.transformCount);
        m_offsetDataLayoutHandle = DataLayoutHandle(originalSizeInfo.datalayoutCount);
        m_offsetDataInstanceHandle = DataInstanceHandle(originalSizeInfo.datainstanceCount);
        m_offsetUniformBufferHandle = UniformBufferHandle(originalSizeInfo.uniformBufferCount);
        m_offsetTextureSamplerHandle = TextureSamplerHandle(originalSizeInfo.textureSamplerCount);
        m_offsetRenderGroupHandle = RenderGroupHandle(originalSizeInfo.renderGroupCount);
        m_offsetRenderPassHandle = RenderPassHandle(originalSizeInfo.renderPassCount);
        m_offsetBlitPassHandle = BlitPassHandle(originalSizeInfo.blitPassCount);
        m_offsetPickableObjectHandle = PickableObjectHandle(originalSizeInfo.pickableObjectCount);
        m_offsetRenderTargetHandle = RenderTargetHandle(originalSizeInfo.renderTargetCount);
        m_offsetRenderBufferHandle = RenderBufferHandle(originalSizeInfo.renderBufferCount);
        m_offsetDataBufferHandle = DataBufferHandle(originalSizeInfo.dataBufferCount);
        m_offsetTextureBufferHandle = TextureBufferHandle(originalSizeInfo.textureBufferCount);
        m_offsetDataSlotHandle = DataSlotHandle(originalSizeInfo.dataSlotCount);
        m_offsetSceneReferenceHandle = SceneReferenceHandle(originalSizeInfo.sceneReferenceCount);
    }

    const std::string& MergeScene::getName() const
    {
        return m_originalScene.getName();
    }

    SceneId MergeScene::getSceneId() const
    {
        return m_originalScene.getSceneId();
    }

    void MergeScene::setEffectTimeSync(FlushTime::Clock::time_point /*t*/)
    {
        // not set by a scene action
    }

    ERenderBackendCompatibility MergeScene::getRenderBackendCompatibility() const
    {
        return m_originalScene.getRenderBackendCompatibility();
    }

    EVulkanAPIVersion MergeScene::getVulkanAPIVersion() const
    {
        return m_originalScene.getVulkanAPIVersion();
    }

    ESPIRVVersion MergeScene::getSPIRVVersion() const
    {
        return m_originalScene.getSPIRVVersion();
    }

    FlushTime::Clock::time_point MergeScene::getEffectTimeSync() const
    {
        return m_originalScene.getEffectTimeSync();
    }

    uint32_t MergeScene::getRenderableCount() const
    {
        return m_originalScene.getRenderableCount();
    }

    RenderableHandle MergeScene::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderableHandle actualHandle = m_originalScene.allocateRenderable(getMappedHandle(nodeHandle), mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderable(RenderableHandle renderableHandle)
    {
        assert(false);
        m_originalScene.releaseRenderable(getMappedHandle(renderableHandle));
    }

    bool MergeScene::isRenderableAllocated(RenderableHandle renderableHandle) const
    {
        return m_originalScene.isRenderableAllocated(getMappedHandle(renderableHandle));
    }

    void MergeScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        m_originalScene.setRenderableDataInstance(getMappedHandle(renderableHandle), slot, getMappedHandle(newDataInstance));
    }

    void MergeScene::setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex)
    {
        m_originalScene.setRenderableStartIndex(getMappedHandle(renderableHandle), startIndex);
    }

    void MergeScene::setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount)
    {
        m_originalScene.setRenderableIndexCount(getMappedHandle(renderableHandle), indexCount);
    }

    void MergeScene::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        m_originalScene.setRenderableRenderState(getMappedHandle(renderableHandle), getMappedHandle(stateHandle));
    }

    void MergeScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible)
    {
        m_originalScene.setRenderableVisibility(getMappedHandle(renderableHandle), visible);
    }

    void MergeScene::setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount)
    {
        m_originalScene.setRenderableInstanceCount(getMappedHandle(renderableHandle), instanceCount);
    }

    void MergeScene::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        m_originalScene.setRenderableStartVertex(getMappedHandle(renderableHandle), startVertex);
    }

    const Renderable& MergeScene::getRenderable(RenderableHandle renderableHandle) const
    {
        return m_originalScene.getRenderable(getMappedHandle(renderableHandle));
    }

    uint32_t MergeScene::getRenderStateCount() const
    {
        return m_originalScene.getRenderStateCount();
    }

    RenderStateHandle MergeScene::allocateRenderState(RenderStateHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderStateHandle actualHandle = m_originalScene.allocateRenderState(mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderState(RenderStateHandle stateHandle)
    {
        assert(false);
        m_originalScene.releaseRenderState(getMappedHandle(stateHandle));
    }

    bool MergeScene::isRenderStateAllocated(RenderStateHandle stateHandle) const
    {
        return m_originalScene.isRenderStateAllocated(getMappedHandle(stateHandle));
    }

    void MergeScene::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        m_originalScene.setRenderStateBlendFactors(getMappedHandle(stateHandle), srcColor, destColor, srcAlpha, destAlpha);
    }

    void MergeScene::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        m_originalScene.setRenderStateBlendOperations(getMappedHandle(stateHandle), operationColor, operationAlpha);
    }

    void MergeScene::setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color)
    {
        m_originalScene.setRenderStateBlendColor(getMappedHandle(stateHandle), color);
    }

    void MergeScene::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        m_originalScene.setRenderStateCullMode(getMappedHandle(stateHandle), cullMode);
    }

    void MergeScene::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        m_originalScene.setRenderStateDrawMode(getMappedHandle(stateHandle), drawMode);
    }

    void MergeScene::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        m_originalScene.setRenderStateDepthFunc(getMappedHandle(stateHandle), func);
    }

    void MergeScene::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        m_originalScene.setRenderStateDepthWrite(getMappedHandle(stateHandle), flag);
    }

    void MergeScene::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        m_originalScene.setRenderStateScissorTest(getMappedHandle(stateHandle), flag, region);
    }

    void MergeScene::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        m_originalScene.setRenderStateStencilFunc(getMappedHandle(stateHandle), func, ref, mask);
    }

    void MergeScene::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        m_originalScene.setRenderStateStencilOps(getMappedHandle(stateHandle), sfail, dpfail, dppass);
    }

    void MergeScene::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        m_originalScene.setRenderStateColorWriteMask(getMappedHandle(stateHandle), colorMask);
    }

    const RenderState& MergeScene::getRenderState(RenderStateHandle stateHandle) const
    {
        return m_originalScene.getRenderState(getMappedHandle(stateHandle));
    }

    uint32_t MergeScene::getCameraCount() const
    {
        return m_originalScene.getCameraCount();
    }

    CameraHandle MergeScene::allocateCamera(ECameraProjectionType projType, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const CameraHandle actualHandle = m_originalScene.allocateCamera(projType, getMappedHandle(nodeHandle), getMappedHandle(dataInstance), mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseCamera(CameraHandle cameraHandle)
    {
        assert(false);
        m_originalScene.releaseCamera(getMappedHandle(cameraHandle));
    }

    bool MergeScene::isCameraAllocated(CameraHandle handle) const
    {
        return m_originalScene.isCameraAllocated(getMappedHandle(handle));
    }

    const Camera& MergeScene::getCamera(CameraHandle cameraHandle) const
    {
        return m_originalScene.getCamera(getMappedHandle(cameraHandle));
    }

    uint32_t MergeScene::getNodeCount() const
    {
        return m_originalScene.getNodeCount();
    }

    NodeHandle MergeScene::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const NodeHandle actualHandle = m_originalScene.allocateNode(childrenCount, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseNode(NodeHandle nodeHandle)
    {
        assert(false);
        m_originalScene.releaseNode(getMappedHandle(nodeHandle));
    }

    bool MergeScene::isNodeAllocated(NodeHandle nodeHandle) const
    {
        return m_originalScene.isNodeAllocated(getMappedHandle(nodeHandle));
    }

    uint32_t MergeScene::getTransformCount() const
    {
        return m_originalScene.getTransformCount();
    }

    TransformHandle MergeScene::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const TransformHandle actualHandle = m_originalScene.allocateTransform(getMappedHandle(nodeHandle), mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseTransform(TransformHandle transform)
    {
        assert(false);
        m_originalScene.releaseTransform(getMappedHandle(transform));
    }

    bool MergeScene::isTransformAllocated(TransformHandle transformHandle) const
    {
        return m_originalScene.isTransformAllocated(getMappedHandle(transformHandle));
    }

    NodeHandle MergeScene::getParent(NodeHandle nodeHandle) const
    {
        return m_originalScene.getParent(getMappedHandle(nodeHandle));
    }

    void MergeScene::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        m_originalScene.addChildToNode(getMappedHandle(parent), getMappedHandle(child));
    }

    void MergeScene::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        m_originalScene.removeChildFromNode(getMappedHandle(parent), getMappedHandle(child));
    }

    uint32_t MergeScene::getChildCount(NodeHandle parent) const
    {
        return m_originalScene.getChildCount(getMappedHandle(parent));
    }

    NodeHandle MergeScene::getChild(NodeHandle parent, uint32_t childNumber) const
    {
        return m_originalScene.getChild(getMappedHandle(parent), childNumber);
    }

    NodeHandle MergeScene::getTransformNode(TransformHandle handle) const
    {
        return m_originalScene.getTransformNode(getMappedHandle(handle));
    }

    const glm::vec3& MergeScene::getTranslation(TransformHandle handle) const
    {
        return m_originalScene.getTranslation(getMappedHandle(handle));
    }

    const glm::vec4& MergeScene::getRotation(TransformHandle handle) const
    {
        return m_originalScene.getRotation(getMappedHandle(handle));
    }

    ERotationType MergeScene::getRotationType(TransformHandle handle) const
    {
        return m_originalScene.getRotationType(getMappedHandle(handle));
    }

    const glm::vec3& MergeScene::getScaling(TransformHandle handle) const
    {
        return m_originalScene.getScaling(getMappedHandle(handle));
    }

    void MergeScene::setTranslation(TransformHandle handle, const glm::vec3& translation)
    {
        m_originalScene.setTranslation(getMappedHandle(handle), translation);
    }

    void MergeScene::setRotation(TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType)
    {
        m_originalScene.setRotation(getMappedHandle(handle), rotation, rotationType);
    }

    void MergeScene::setScaling(TransformHandle handle, const glm::vec3& scaling)
    {
        m_originalScene.setScaling(getMappedHandle(handle), scaling);
    }

    uint32_t MergeScene::getDataLayoutCount() const
    {
        return m_originalScene.getDataLayoutCount();
    }

    DataLayoutHandle MergeScene::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const DataLayoutHandle actualHandle = m_originalScene.allocateDataLayout(dataFields, effectHash, mappedHandle);
        // because of data layout caching the returned handle might be different
        if (m_mapping.hasMapping(handle))
        {
            assert(m_mapping.getMapping(handle) == actualHandle);
            return actualHandle;
        }
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        assert(false);
        m_originalScene.releaseDataLayout(getMappedHandle(layoutHandle));
    }

    bool MergeScene::isDataLayoutAllocated(DataLayoutHandle layoutHandle) const
    {
        return m_originalScene.isDataLayoutAllocated(getMappedHandle(layoutHandle));
    }

    const DataLayout& MergeScene::getDataLayout(DataLayoutHandle layoutHandle) const
    {
        return m_originalScene.getDataLayout(getMappedHandle(layoutHandle));
    }

    uint32_t MergeScene::getDataInstanceCount() const
    {
        return m_originalScene.getDataInstanceCount();
    }

    DataInstanceHandle MergeScene::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const auto actualHandle = m_originalScene.allocateDataInstance(getMappedHandle(finishedLayoutHandle), mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseDataInstance(DataInstanceHandle containerHandle)
    {
        assert(false);
        m_originalScene.releaseDataInstance(getMappedHandle(containerHandle));
    }

    bool MergeScene::isDataInstanceAllocated(DataInstanceHandle containerHandle) const
    {
        return m_originalScene.isDataInstanceAllocated(getMappedHandle(containerHandle));
    }

    DataLayoutHandle MergeScene::getLayoutOfDataInstance(DataInstanceHandle containerHandle) const
    {
        return m_originalScene.getLayoutOfDataInstance(getMappedHandle(containerHandle));
    }

    const float* MergeScene::getDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataFloatArray(getMappedHandle(containerHandle), field);
    }

    const glm::vec2* MergeScene::getDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector2fArray(getMappedHandle(containerHandle), field);
    }

    const glm::vec3* MergeScene::getDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector3fArray(getMappedHandle(containerHandle), field);
    }

    const glm::vec4* MergeScene::getDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector4fArray(getMappedHandle(containerHandle), field);
    }

    const bool* MergeScene::getDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataBooleanArray(getMappedHandle(containerHandle), field);
    }

    const int32_t* MergeScene::getDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataIntegerArray(getMappedHandle(containerHandle), field);
    }

    const glm::mat2* MergeScene::getDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataMatrix22fArray(getMappedHandle(containerHandle), field);
    }

    const glm::mat3* MergeScene::getDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataMatrix33fArray(getMappedHandle(containerHandle), field);
    }

    const glm::mat4* MergeScene::getDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataMatrix44fArray(getMappedHandle(containerHandle), field);
    }

    const glm::ivec2* MergeScene::getDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector2iArray(getMappedHandle(containerHandle), field);
    }

    const glm::ivec3* MergeScene::getDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector3iArray(getMappedHandle(containerHandle), field);
    }

    const glm::ivec4* MergeScene::getDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataVector4iArray(getMappedHandle(containerHandle), field);
    }

    const ResourceField& MergeScene::getDataResource(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataResource(getMappedHandle(containerHandle), field);
    }

    TextureSamplerHandle MergeScene::getDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataTextureSamplerHandle(getMappedHandle(containerHandle), field);
    }

    DataInstanceHandle MergeScene::getDataReference(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataReference(getMappedHandle(containerHandle), field);
    }

    UniformBufferHandle MergeScene::getDataUniformBuffer(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataUniformBuffer(getMappedHandle(containerHandle), field);
    }

    float MergeScene::getDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleFloat(getMappedHandle(containerHandle), field);
    }

    const glm::vec2& MergeScene::getDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector2f(getMappedHandle(containerHandle), field);
    }

    const glm::vec3& MergeScene::getDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector3f(getMappedHandle(containerHandle), field);
    }

    const glm::vec4& MergeScene::getDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector4f(getMappedHandle(containerHandle), field);
    }

    bool MergeScene::getDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleBoolean(getMappedHandle(containerHandle), field);
    }

    int32_t MergeScene::getDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleInteger(getMappedHandle(containerHandle), field);
    }

    const glm::mat2& MergeScene::getDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleMatrix22f(getMappedHandle(containerHandle), field);
    }

    const glm::mat3& MergeScene::getDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleMatrix33f(getMappedHandle(containerHandle), field);
    }

    const glm::mat4& MergeScene::getDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleMatrix44f(getMappedHandle(containerHandle), field);
    }

    const glm::ivec2& MergeScene::getDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector2i(getMappedHandle(containerHandle), field);
    }

    const glm::ivec3& MergeScene::getDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector3i(getMappedHandle(containerHandle), field);
    }

    const glm::ivec4& MergeScene::getDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field) const
    {
        return m_originalScene.getDataSingleVector4i(getMappedHandle(containerHandle), field);
    }

    void MergeScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data)
    {
        m_originalScene.setDataFloatArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data)
    {
        m_originalScene.setDataVector2fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data)
    {
        m_originalScene.setDataVector3fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data)
    {
        m_originalScene.setDataVector4fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data)
    {
        m_originalScene.setDataMatrix22fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data)
    {
        m_originalScene.setDataMatrix33fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data)
    {
        m_originalScene.setDataMatrix44fArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data)
    {
        m_originalScene.setDataBooleanArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data)
    {
        m_originalScene.setDataIntegerArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data)
    {
        m_originalScene.setDataVector2iArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data)
    {
        m_originalScene.setDataVector3iArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data)
    {
        m_originalScene.setDataVector4iArray(getMappedHandle(containerHandle), field, elementCount, data);
    }

    void MergeScene::setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        m_originalScene.setDataResource(getMappedHandle(containerHandle), field, hash, getMappedHandle(dataBuffer), instancingDivisor, offsetWithinElementInBytes, stride);
    }

    void MergeScene::setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        m_originalScene.setDataTextureSamplerHandle(getMappedHandle(containerHandle), field, getMappedHandle(samplerHandle));
    }

    void MergeScene::setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        m_originalScene.setDataReference(getMappedHandle(containerHandle), field, getMappedHandle(dataRef));
    }

    void MergeScene::setDataUniformBuffer(DataInstanceHandle containerHandle, DataFieldHandle field, UniformBufferHandle uniformBufferHandle)
    {
        m_originalScene.setDataUniformBuffer(getMappedHandle(containerHandle), field, getMappedHandle(uniformBufferHandle));
    }

    void MergeScene::setDataSingleFloat(DataInstanceHandle containerHandle, DataFieldHandle field, float data)
    {
        m_originalScene.setDataSingleFloat(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector2f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec2& data)
    {
        m_originalScene.setDataSingleVector2f(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector3f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec3& data)
    {
        m_originalScene.setDataSingleVector3f(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector4f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec4& data)
    {
        m_originalScene.setDataSingleVector4f(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleBoolean(DataInstanceHandle containerHandle, DataFieldHandle field, bool data)
    {
        m_originalScene.setDataSingleBoolean(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleInteger(DataInstanceHandle containerHandle, DataFieldHandle field, int32_t data)
    {
        m_originalScene.setDataSingleInteger(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector2i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec2& data)
    {
        m_originalScene.setDataSingleVector2i(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector3i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec3& data)
    {
        m_originalScene.setDataSingleVector3i(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleVector4i(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec4& data)
    {
        m_originalScene.setDataSingleVector4i(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleMatrix22f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat2& data)
    {
        m_originalScene.setDataSingleMatrix22f(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleMatrix33f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat3& data)
    {
        m_originalScene.setDataSingleMatrix33f(getMappedHandle(containerHandle), field, data);
    }

    void MergeScene::setDataSingleMatrix44f(DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat4& data)
    {
        m_originalScene.setDataSingleMatrix44f(getMappedHandle(containerHandle), field, data);
    }

    TextureSamplerHandle MergeScene::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);

        auto mappedSampler = sampler;
        switch (sampler.contentType)
        {
        case TextureSampler::ContentType::TextureBuffer:
            mappedSampler.contentHandle = getMappedHandle(TextureBufferHandle(sampler.contentHandle)).asMemoryHandle();
            break;
        case TextureSampler::ContentType::RenderBuffer:
        case TextureSampler::ContentType::RenderBufferMS:
            mappedSampler.contentHandle = getMappedHandle(RenderBufferHandle(sampler.contentHandle)).asMemoryHandle();
            break;
        case TextureSampler::ContentType::None:
        case TextureSampler::ContentType::ClientTexture:
        case TextureSampler::ContentType::OffscreenBuffer:
        case TextureSampler::ContentType::StreamBuffer:
        case TextureSampler::ContentType::ExternalTexture:
          break;
        }

        const TextureSamplerHandle actualHandle = m_originalScene.allocateTextureSampler(mappedSampler, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseTextureSampler(TextureSamplerHandle handle)
    {
        assert(false);
        m_originalScene.releaseTextureSampler(getMappedHandle(handle));
    }

    uint32_t MergeScene::getTextureSamplerCount() const
    {
        return m_originalScene.getTextureSamplerCount();
    }

    const TextureSampler& MergeScene::getTextureSampler(TextureSamplerHandle handle) const
    {
        return m_originalScene.getTextureSampler(getMappedHandle(handle));
    }

    bool MergeScene::isTextureSamplerAllocated(TextureSamplerHandle samplerHandle) const
    {
        return m_originalScene.isTextureSamplerAllocated(getMappedHandle(samplerHandle));
    }

    SceneSizeInformation MergeScene::getSceneSizeInformation() const
    {
        return m_originalScene.getSceneSizeInformation();
    }

    RenderGroupHandle MergeScene::allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderGroupHandle actualHandle = m_originalScene.allocateRenderGroup(renderableCount, nestedGroupCount, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        assert(false);
        m_originalScene.releaseRenderGroup(getMappedHandle(groupHandle));
    }

    bool MergeScene::isRenderGroupAllocated(RenderGroupHandle groupHandle) const
    {
        return m_originalScene.isRenderGroupAllocated(getMappedHandle(groupHandle));
    }

    uint32_t MergeScene::getRenderGroupCount() const
    {
        return m_originalScene.getRenderGroupCount();
    }

    void MergeScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        m_originalScene.addRenderableToRenderGroup(getMappedHandle(groupHandle), getMappedHandle(renderableHandle), order);
    }

    void MergeScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        m_originalScene.removeRenderableFromRenderGroup(getMappedHandle(groupHandle), getMappedHandle(renderableHandle));
    }

    void MergeScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        m_originalScene.addRenderGroupToRenderGroup(getMappedHandle(groupHandleParent), getMappedHandle(groupHandleChild), order);
    }

    void MergeScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        m_originalScene.removeRenderGroupFromRenderGroup(getMappedHandle(groupHandleParent), getMappedHandle(groupHandleChild));
    }

    const RenderGroup& MergeScene::getRenderGroup(RenderGroupHandle groupHandle) const
    {
        return m_originalScene.getRenderGroup(getMappedHandle(groupHandle));
    }

    RenderPassHandle MergeScene::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderPassHandle actualHandle = m_originalScene.allocateRenderPass(renderGroupCount, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderPass(RenderPassHandle handle)
    {
        assert(false);
        m_originalScene.releaseRenderPass(getMappedHandle(handle));
    }

    bool MergeScene::isRenderPassAllocated(RenderPassHandle pass) const
    {
        return m_originalScene.isRenderPassAllocated(getMappedHandle(pass));
    }

    uint32_t MergeScene::getRenderPassCount() const
    {
        return m_originalScene.getRenderPassCount();
    }

    void MergeScene::setRenderPassCamera(RenderPassHandle pass, CameraHandle camera)
    {
        m_originalScene.setRenderPassCamera(getMappedHandle(pass), getMappedHandle(camera));
    }

    RenderTargetHandle MergeScene::allocateRenderTarget(RenderTargetHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderTargetHandle actualHandle = m_originalScene.allocateRenderTarget(mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderTarget(RenderTargetHandle targetHandle)
    {
        assert(false);
        m_originalScene.releaseRenderTarget(getMappedHandle(targetHandle));
    }

    void MergeScene::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        m_originalScene.addRenderTargetRenderBuffer(getMappedHandle(targetHandle), getMappedHandle(bufferHandle));
    }

    uint32_t MergeScene::getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const
    {
        return m_originalScene.getRenderTargetRenderBufferCount(getMappedHandle(targetHandle));
    }

    RenderBufferHandle MergeScene::getRenderTargetRenderBuffer(RenderTargetHandle targetHandle, uint32_t bufferIndex) const
    {
        return m_originalScene.getRenderTargetRenderBuffer(getMappedHandle(targetHandle), bufferIndex);
    }

    RenderBufferHandle MergeScene::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const RenderBufferHandle actualHandle = m_originalScene.allocateRenderBuffer(renderBuffer, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseRenderBuffer(RenderBufferHandle handle)
    {
        assert(false);
        m_originalScene.releaseRenderBuffer(getMappedHandle(handle));
    }

    void MergeScene::setRenderBufferProperties(RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        m_originalScene.setRenderBufferProperties(getMappedHandle(handle), width, height, sampleCount);
    }

    bool MergeScene::isRenderBufferAllocated(RenderBufferHandle handle) const
    {
        return m_originalScene.isRenderBufferAllocated(getMappedHandle(handle));
    }

    uint32_t MergeScene::getRenderBufferCount() const
    {
        return m_originalScene.getRenderBufferCount();
    }

    const RenderBuffer& MergeScene::getRenderBuffer(RenderBufferHandle handle) const
    {
        return m_originalScene.getRenderBuffer(getMappedHandle(handle));
    }

    DataBufferHandle MergeScene::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const DataBufferHandle actualHandle = m_originalScene.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseDataBuffer(DataBufferHandle handle)
    {
        assert(false);
        m_originalScene.releaseDataBuffer(getMappedHandle(handle));
    }

    uint32_t MergeScene::getDataBufferCount() const
    {
        return m_originalScene.getDataBufferCount();
    }

    void MergeScene::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        m_originalScene.updateDataBuffer(getMappedHandle(handle), offsetInBytes, dataSizeInBytes, data);
    }

    bool MergeScene::isDataBufferAllocated(DataBufferHandle handle) const
    {
        return m_originalScene.isDataBufferAllocated(getMappedHandle(handle));
    }

    const GeometryDataBuffer& MergeScene::getDataBuffer(DataBufferHandle handle) const
    {
        return m_originalScene.getDataBuffer(getMappedHandle(handle));
    }

    UniformBufferHandle MergeScene::allocateUniformBuffer(uint32_t size, UniformBufferHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const auto actualHandle = m_originalScene.allocateUniformBuffer(size, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseUniformBuffer(UniformBufferHandle uniformBufferHandle)
    {
        assert(false);
        m_originalScene.releaseUniformBuffer(getMappedHandle(uniformBufferHandle));
    }

    void MergeScene::updateUniformBuffer(UniformBufferHandle uniformBufferHandle, uint32_t offset, uint32_t size, const std::byte* data)
    {
        m_originalScene.updateUniformBuffer(getMappedHandle(uniformBufferHandle), offset, size, data);
    }

    bool MergeScene::isUniformBufferAllocated(UniformBufferHandle uniformBufferHandle) const
    {
        return m_originalScene.isUniformBufferAllocated(getMappedHandle(uniformBufferHandle));
    }

    uint32_t MergeScene::getUniformBufferCount() const
    {
        return m_originalScene.getUniformBufferCount();
    }

    const UniformBuffer& MergeScene::getUniformBuffer(UniformBufferHandle uniformBufferHandle) const
    {
        return m_originalScene.getUniformBuffer(getMappedHandle(uniformBufferHandle));
    }

    TextureBufferHandle MergeScene::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const TextureBufferHandle actualHandle = m_originalScene.allocateTextureBuffer(textureFormat, mipMapDimensions, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        assert(false);
        m_originalScene.releaseTextureBuffer(getMappedHandle(handle));
    }

    uint32_t MergeScene::getTextureBufferCount() const
    {
        return m_originalScene.getTextureBufferCount();
    }

    void MergeScene::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        m_originalScene.updateTextureBuffer(getMappedHandle(handle), mipLevel, x, y, width, height, data);
    }

    const TextureBuffer& MergeScene::getTextureBuffer(TextureBufferHandle handle) const
    {
        return m_originalScene.getTextureBuffer(getMappedHandle(handle));
    }

    bool MergeScene::isTextureBufferAllocated(TextureBufferHandle handle) const
    {
        return m_originalScene.isTextureBufferAllocated(getMappedHandle(handle));
    }

    DataSlotHandle MergeScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const DataSlot mappedDataSlot{
            dataSlot.type,
            dataSlot.id,
            getMappedHandle(dataSlot.attachedNode),
            getMappedHandle(dataSlot.attachedDataReference),
            dataSlot.attachedTexture,
            getMappedHandle(dataSlot.attachedTextureSampler)
        };
        const DataSlotHandle actualHandle = m_originalScene.allocateDataSlot(mappedDataSlot, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        m_originalScene.setDataSlotTexture(getMappedHandle(handle), texture);
    }

    void MergeScene::releaseDataSlot(DataSlotHandle handle)
    {
        assert(false);
        m_originalScene.releaseDataSlot(getMappedHandle(handle));
    }

    bool MergeScene::isDataSlotAllocated(DataSlotHandle handle) const
    {
        return m_originalScene.isDataSlotAllocated(getMappedHandle(handle));
    }

    uint32_t MergeScene::getDataSlotCount() const
    {
        return m_originalScene.getDataSlotCount();
    }

    const DataSlot& MergeScene::getDataSlot(DataSlotHandle handle) const
    {
        return m_originalScene.getDataSlot(getMappedHandle(handle));
    }

    SceneReferenceHandle MergeScene::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const auto actualHandle = m_originalScene.allocateSceneReference(sceneId, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseSceneReference(SceneReferenceHandle handle)
    {
        assert(false);
        m_originalScene.releaseSceneReference(getMappedHandle(handle));
    }

    void MergeScene::requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state)
    {
        m_originalScene.requestSceneReferenceState(getMappedHandle(handle), state);
    }

    void MergeScene::requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable)
    {
        m_originalScene.requestSceneReferenceFlushNotifications(getMappedHandle(handle), enable);
    }

    void MergeScene::setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder)
    {
        m_originalScene.setSceneReferenceRenderOrder(getMappedHandle(handle), renderOrder);
    }

    bool MergeScene::isSceneReferenceAllocated(SceneReferenceHandle handle) const
    {
        return m_originalScene.isSceneReferenceAllocated(getMappedHandle(handle));
    }

    uint32_t MergeScene::getSceneReferenceCount() const
    {
        return m_originalScene.getSceneReferenceCount();
    }

    const SceneReference& MergeScene::getSceneReference(SceneReferenceHandle handle) const
    {
        return m_originalScene.getSceneReference(getMappedHandle(handle));
    }

    void MergeScene::setRenderPassClearFlag(RenderPassHandle handle, ClearFlags clearFlag)
    {
        m_originalScene.setRenderPassClearFlag(getMappedHandle(handle), clearFlag);
    }

    void MergeScene::setRenderPassClearColor(RenderPassHandle handle, const glm::vec4& clearColor)
    {
        m_originalScene.setRenderPassClearColor(getMappedHandle(handle), clearColor);
    }

    void MergeScene::setRenderPassRenderTarget(RenderPassHandle pass, RenderTargetHandle targetHandle)
    {
        m_originalScene.setRenderPassRenderTarget(getMappedHandle(pass), getMappedHandle(targetHandle));
    }

    void MergeScene::setRenderPassRenderOrder(RenderPassHandle pass, int32_t renderOrder)
    {
        m_originalScene.setRenderPassRenderOrder(getMappedHandle(pass), renderOrder);
    }

    void MergeScene::setRenderPassEnabled(RenderPassHandle pass, bool isEnabled)
    {
        m_originalScene.setRenderPassEnabled(getMappedHandle(pass), isEnabled);
    }

    void MergeScene::setRenderPassRenderOnce(RenderPassHandle pass, bool enable)
    {
        m_originalScene.setRenderPassRenderOnce(getMappedHandle(pass), enable);
    }

    void MergeScene::retriggerRenderPassRenderOnce(RenderPassHandle pass)
    {
        m_originalScene.retriggerRenderPassRenderOnce(getMappedHandle(pass));
    }

    void MergeScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        m_originalScene.addRenderGroupToRenderPass(getMappedHandle(passHandle), getMappedHandle(groupHandle), order);
    }

    void MergeScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        m_originalScene.removeRenderGroupFromRenderPass(getMappedHandle(passHandle), getMappedHandle(groupHandle));
    }

    const RenderPass& MergeScene::getRenderPass(RenderPassHandle passHandle) const
    {
        return m_originalScene.getRenderPass(getMappedHandle(passHandle));
    }

    PickableObjectHandle MergeScene::allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const PickableObjectHandle actualHandle = m_originalScene.allocatePickableObject(getMappedHandle(geometryHandle), getMappedHandle(nodeHandle), id, mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releasePickableObject(PickableObjectHandle pickableHandle)
    {
        assert(false);
        m_originalScene.releasePickableObject(getMappedHandle(pickableHandle));
    }

    bool MergeScene::isPickableObjectAllocated(PickableObjectHandle pickableHandle) const
    {
        return m_originalScene.isPickableObjectAllocated(getMappedHandle(pickableHandle));
    }

    uint32_t MergeScene::getPickableObjectCount() const
    {
        return m_originalScene.getPickableObjectCount();
    }

    void MergeScene::setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id)
    {
        m_originalScene.setPickableObjectId(getMappedHandle(pickableHandle), id);
    }

    void MergeScene::setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle)
    {
        m_originalScene.setPickableObjectCamera(getMappedHandle(pickableHandle), getMappedHandle(cameraHandle));
    }

    void MergeScene::setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled)
    {
        m_originalScene.setPickableObjectEnabled(getMappedHandle(pickableHandle), isEnabled);
    }

    const PickableObject& MergeScene::getPickableObject(PickableObjectHandle pickableHandle) const
    {
        return m_originalScene.getPickableObject(getMappedHandle(pickableHandle));
    }

    BlitPassHandle MergeScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle handle)
    {
        assert(handle.isValid());
        const auto mappedHandle = getMappedHandle(handle);
        const BlitPassHandle actualHandle = m_originalScene.allocateBlitPass(getMappedHandle(sourceRenderBufferHandle), getMappedHandle(destinationRenderBufferHandle), mappedHandle);
        assert(mappedHandle == actualHandle);
        return addMapping(handle, actualHandle);
    }

    void MergeScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        assert(false);
        m_originalScene.releaseBlitPass(getMappedHandle(passHandle));
    }

    bool MergeScene::isBlitPassAllocated(BlitPassHandle passHandle) const
    {
        return m_originalScene.isBlitPassAllocated(getMappedHandle(passHandle));
    }

    uint32_t MergeScene::getBlitPassCount() const
    {
        return m_originalScene.getBlitPassCount();
    }

    void MergeScene::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        m_originalScene.setBlitPassRenderOrder(getMappedHandle(passHandle), renderOrder);
    }

    void MergeScene::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        m_originalScene.setBlitPassEnabled(getMappedHandle(passHandle), isEnabled);
    }

    void MergeScene::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        m_originalScene.setBlitPassRegions(getMappedHandle(passHandle), sourceRegion, destinationRegion);
    }

    const BlitPass& MergeScene::getBlitPass(BlitPassHandle passHandle) const
    {
        return m_originalScene.getBlitPass(getMappedHandle(passHandle));
    }

    bool MergeScene::isRenderTargetAllocated(RenderTargetHandle targetHandle) const
    {
        return m_originalScene.isRenderTargetAllocated(getMappedHandle(targetHandle));
    }

    uint32_t MergeScene::getRenderTargetCount() const
    {
        return m_originalScene.getRenderTargetCount();
    }

    void MergeScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        const SceneSizeInformation newSizeInfo{
            sizeInfo.nodeCount + m_offsetNodeHandle.asMemoryHandle(),
            sizeInfo.cameraCount + m_offsetCameraHandle.asMemoryHandle(),
            sizeInfo.transformCount + m_offsetTransformHandle.asMemoryHandle(),
            sizeInfo.renderableCount + m_offsetRenderableHandle.asMemoryHandle(),
            sizeInfo.renderStateCount + m_offsetRenderStateHandle.asMemoryHandle(),
            sizeInfo.datalayoutCount + m_offsetDataLayoutHandle.asMemoryHandle(),
            sizeInfo.datainstanceCount + m_offsetDataInstanceHandle.asMemoryHandle(),
            sizeInfo.uniformBufferCount + m_offsetUniformBufferHandle.asMemoryHandle(),
            sizeInfo.renderGroupCount + m_offsetRenderGroupHandle.asMemoryHandle(),
            sizeInfo.renderPassCount + m_offsetRenderPassHandle.asMemoryHandle(),
            sizeInfo.blitPassCount + m_offsetBlitPassHandle.asMemoryHandle(),
            sizeInfo.renderTargetCount + m_offsetRenderTargetHandle.asMemoryHandle(),
            sizeInfo.renderBufferCount + m_offsetRenderBufferHandle.asMemoryHandle(),
            sizeInfo.textureSamplerCount + m_offsetTextureSamplerHandle.asMemoryHandle(),
            sizeInfo.dataSlotCount + m_offsetDataSlotHandle.asMemoryHandle(),
            sizeInfo.dataBufferCount + m_offsetDataBufferHandle.asMemoryHandle(),
            sizeInfo.textureBufferCount + m_offsetTextureBufferHandle.asMemoryHandle(),
            sizeInfo.pickableObjectCount + m_offsetPickableObjectHandle.asMemoryHandle(),
            sizeInfo.sceneReferenceCount + m_offsetSceneReferenceHandle.asMemoryHandle(),
        };

        m_originalScene.preallocateSceneSize(newSizeInfo);
    }

    template<>
    inline TypedMemoryHandle<RenderableHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderableHandle;
    }

    template<>
    inline TypedMemoryHandle<StateHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderStateHandle;
    }

    template<>
    inline TypedMemoryHandle<CameraHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetCameraHandle;
    }

    template<>
    inline TypedMemoryHandle<NodeHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetNodeHandle;
    }

    template<>
    inline TypedMemoryHandle<TransformHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetTransformHandle;
    }

    template<>
    inline TypedMemoryHandle<DataLayoutHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetDataLayoutHandle;
    }

    template<>
    inline TypedMemoryHandle<DataInstanceHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetDataInstanceHandle;
    }

    template<>
    inline TypedMemoryHandle<UniformBufferHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetUniformBufferHandle;
    }

    template<>
    inline TypedMemoryHandle<TextureSamplerHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetTextureSamplerHandle;
    }

    template<>
    inline TypedMemoryHandle<RenderGroupHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderGroupHandle;
    }

    template<>
    inline TypedMemoryHandle<RenderPassHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderPassHandle;
    }

    template<>
    inline TypedMemoryHandle<BlitPassHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetBlitPassHandle;
    }

    template<>
    inline TypedMemoryHandle<PickableObjectTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetPickableObjectHandle;
    }

    template<>
    inline TypedMemoryHandle<RenderTargetHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderTargetHandle;
    }

    template<>
    inline TypedMemoryHandle<RenderBufferHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetRenderBufferHandle;
    }

    template<>
    inline TypedMemoryHandle<DataBufferHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetDataBufferHandle;
    }

    template<>
    inline TypedMemoryHandle<TextureBufferHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetTextureBufferHandle;
    }

    template<>
    inline TypedMemoryHandle<DataSlotHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetDataSlotHandle;
    }

    template<>
    inline TypedMemoryHandle<SceneReferenceHandleTag> MergeScene::getOffsetHandle() const
    {
        return m_offsetSceneReferenceHandle;
    }


    template<typename T>
    [[nodiscard]] inline TypedMemoryHandle<T> MergeScene::getMappedHandle(TypedMemoryHandle<T> handle) const
    {
        if (handle.isValid())
        {
            const auto offset = getOffsetHandle<T>();
            assert(std::numeric_limits<typename TypedMemoryHandle<T>::Type>::max() - offset.asMemoryHandle() > handle.asMemoryHandle());
            return handle + offset.asMemoryHandle();
        }
        return TypedMemoryHandle<T>::Invalid();
    }

    template<>
    [[nodiscard]] inline TypedMemoryHandle<DataLayoutHandleTag> MergeScene::getMappedHandle(TypedMemoryHandle<DataLayoutHandleTag> handle) const
    {
        if (handle.isValid())
        {
            const auto mappedHandle = m_mapping.getMapping(handle);
            if (mappedHandle.isValid())
            {
                return mappedHandle;
            }
            const auto offset = getOffsetHandle<DataLayoutHandleTag>();
            assert(std::numeric_limits<typename TypedMemoryHandle<DataLayoutHandleTag>::Type>::max() - offset.asMemoryHandle() > handle.asMemoryHandle());
            return handle + offset.asMemoryHandle();
        }
        return TypedMemoryHandle<DataLayoutHandleTag>::Invalid();
    }

    template<typename T>
    [[nodiscard]] inline TypedMemoryHandle<T> MergeScene::addMapping(TypedMemoryHandle<T> handle, TypedMemoryHandle<T> newHandle)
    {
        if (handle.isValid())
        {
            assert(newHandle.isValid());
            m_mapping.addMapping(handle, newHandle);
        }
        return newHandle;
    }
}
