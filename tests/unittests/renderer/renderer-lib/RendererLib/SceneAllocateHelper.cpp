//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneAllocateHelper.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"

namespace ramses::internal
{
    SceneAllocateHelper::SceneAllocateHelper(IScene& scene)
        : m_scene(scene)
    {
    }

    template <typename HANDLE>
    uint32_t& getObjectCount(SceneSizeInformation& sizeInfo)
    {
        assert(false);
        return sizeInfo.nodeCount;
    }
    template <> uint32_t& getObjectCount<NodeHandle>          (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.nodeCount;
    }
    template <> uint32_t& getObjectCount<CameraHandle>        (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.cameraCount;
    }
    template <> uint32_t& getObjectCount<TransformHandle>     (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.transformCount;
    }
    template <> uint32_t& getObjectCount<RenderableHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderableCount;
    }
    template <> uint32_t& getObjectCount<RenderStateHandle>   (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderStateCount;
    }
    template <> uint32_t& getObjectCount<DataLayoutHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.datalayoutCount;
    }
    template <> uint32_t& getObjectCount<DataInstanceHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.datainstanceCount;
    }
    template <> uint32_t& getObjectCount<RenderGroupHandle>   (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderGroupCount;
    }
    template <> uint32_t& getObjectCount<RenderPassHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderPassCount;
    }
    template <> uint32_t& getObjectCount<BlitPassHandle>      (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.blitPassCount;
    }
    template <> uint32_t& getObjectCount<RenderTargetHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderTargetCount;
    }
    template <> uint32_t& getObjectCount<RenderBufferHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderBufferCount;
    }
    template <> uint32_t& getObjectCount<TextureSamplerHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.textureSamplerCount;
    }
    template <> uint32_t& getObjectCount<DataSlotHandle>      (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.dataSlotCount;
    }
    template <> uint32_t& getObjectCount<DataBufferHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.dataBufferCount;
    }
    template <> uint32_t& getObjectCount<UniformBufferHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.uniformBufferCount;
    }
    template <> uint32_t& getObjectCount<TextureBufferHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.textureBufferCount;
    }
    template <> uint32_t& getObjectCount<PickableObjectHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.pickableObjectCount;
    }
    template <> uint32_t& getObjectCount<SceneReferenceHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.sceneReferenceCount;
    }

    template <typename HANDLE>
    HANDLE SceneAllocateHelper::preallocateHandle(HANDLE handle)
    {
        HANDLE actualHandle = handle;
        SceneSizeInformation sizeInfo = m_scene.getSceneSizeInformation();
        uint32_t& objectCount = getObjectCount<HANDLE>(sizeInfo);

        if (!actualHandle.isValid())
        {
            actualHandle = HANDLE(objectCount++);
            m_scene.preallocateSceneSize(sizeInfo);
        }
        else if (actualHandle >= objectCount)
        {
            objectCount = actualHandle.asMemoryHandle() + 1u;
            m_scene.preallocateSceneSize(sizeInfo);
        }

        return actualHandle;
    }

    RenderableHandle SceneAllocateHelper::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        return m_scene.allocateRenderable(nodeHandle, preallocateHandle(handle));
    }

    RenderStateHandle SceneAllocateHelper::allocateRenderState(RenderStateHandle handle)
    {
        return m_scene.allocateRenderState(preallocateHandle(handle));
    }

    CameraHandle SceneAllocateHelper::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        return m_scene.allocateCamera(type, nodeHandle, dataInstance, preallocateHandle(handle));
    }

    NodeHandle SceneAllocateHelper::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        return m_scene.allocateNode(childrenCount, preallocateHandle(handle));
    }

    TransformHandle SceneAllocateHelper::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        return m_scene.allocateTransform(nodeHandle, preallocateHandle(handle));
    }

    DataLayoutHandle SceneAllocateHelper::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        return m_scene.allocateDataLayout(dataFields, effectHash, preallocateHandle(handle));
    }

    DataInstanceHandle SceneAllocateHelper::allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle handle)
    {
        return m_scene.allocateDataInstance(finishedLayoutHandle, preallocateHandle(handle));
    }

    TextureSamplerHandle SceneAllocateHelper::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        return m_scene.allocateTextureSampler(sampler, preallocateHandle(handle));
    }

    RenderGroupHandle SceneAllocateHelper::allocateRenderGroup(RenderGroupHandle handle)
    {
        return m_scene.allocateRenderGroup(0u, 0u, preallocateHandle(handle));
    }

    RenderPassHandle SceneAllocateHelper::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle)
    {
        return m_scene.allocateRenderPass(renderGroupCount, preallocateHandle(handle));
    }

    BlitPassHandle SceneAllocateHelper::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle handle)
    {
        return m_scene.allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, preallocateHandle(handle));
    }

    RenderTargetHandle SceneAllocateHelper::allocateRenderTarget(RenderTargetHandle handle)
    {
        return m_scene.allocateRenderTarget(preallocateHandle(handle));
    }

    RenderBufferHandle SceneAllocateHelper::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        return m_scene.allocateRenderBuffer(renderBuffer, preallocateHandle(handle));
    }

    DataSlotHandle SceneAllocateHelper::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        return m_scene.allocateDataSlot(dataSlot, preallocateHandle(handle));
    }

    DataBufferHandle SceneAllocateHelper::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        return m_scene.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, preallocateHandle(handle));
    }

    UniformBufferHandle SceneAllocateHelper::allocateUniformBuffer(uint32_t size, UniformBufferHandle handle)
    {
        return m_scene.allocateUniformBuffer(size, preallocateHandle(handle));
    }

    TextureBufferHandle SceneAllocateHelper::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        return m_scene.allocateTextureBuffer(textureFormat, mipMapDimensions, preallocateHandle(handle));
    }

    PickableObjectHandle SceneAllocateHelper::allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle)
    {
        return m_scene.allocatePickableObject(geometryHandle, nodeHandle, id, preallocateHandle(pickableHandle));
    }

    SceneReferenceHandle SceneAllocateHelper::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        return m_scene.allocateSceneReference(sceneId, preallocateHandle(handle));
    }
}
