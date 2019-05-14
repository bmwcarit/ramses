//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneAllocateHelper.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/RenderBuffer.h"

namespace ramses_internal
{
    SceneAllocateHelper::SceneAllocateHelper(IScene& scene)
        : m_scene(scene)
    {
    }

    template <typename HANDLE>
    UInt32& getObjectCount(SceneSizeInformation& sizeInfo)
    {
        assert(false);
        return sizeInfo.nodeCount;
    }
    template <> UInt32& getObjectCount<NodeHandle>          (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.nodeCount;
    }
    template <> UInt32& getObjectCount<CameraHandle>        (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.cameraCount;
    }
    template <> UInt32& getObjectCount<TransformHandle>     (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.transformCount;
    }
    template <> UInt32& getObjectCount<RenderableHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderableCount;
    }
    template <> UInt32& getObjectCount<RenderStateHandle>   (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderStateCount;
    }
    template <> UInt32& getObjectCount<DataLayoutHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.datalayoutCount;
    }
    template <> UInt32& getObjectCount<DataInstanceHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.datainstanceCount;
    }
    template <> UInt32& getObjectCount<RenderGroupHandle>   (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderGroupCount;
    }
    template <> UInt32& getObjectCount<RenderPassHandle>    (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderPassCount;
    }
    template <> UInt32& getObjectCount<BlitPassHandle>      (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.blitPassCount;
    }
    template <> UInt32& getObjectCount<RenderTargetHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderTargetCount;
    }
    template <> UInt32& getObjectCount<RenderBufferHandle>  (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.renderBufferCount;
    }
    template <> UInt32& getObjectCount<TextureSamplerHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.textureSamplerCount;
    }
    template <> UInt32& getObjectCount<StreamTextureHandle> (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.streamTextureCount;
    }
    template <> UInt32& getObjectCount<DataSlotHandle>      (SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.dataSlotCount;
    }
    template <> UInt32& getObjectCount<DataBufferHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.dataBufferCount;
    }
    template <> UInt32& getObjectCount<AnimationSystemHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.animationSystemCount;
    }
    template <> UInt32& getObjectCount<TextureBufferHandle>(SceneSizeInformation& sizeInfo)
    {
        return sizeInfo.textureBufferCount;
    }

    template <typename HANDLE>
    HANDLE SceneAllocateHelper::preallocateHandle(HANDLE handle)
    {
        HANDLE actualHandle = handle;
        SceneSizeInformation sizeInfo = m_scene.getSceneSizeInformation();
        UInt32& objectCount = getObjectCount<HANDLE>(sizeInfo);

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

    CameraHandle SceneAllocateHelper::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle)
    {
        return m_scene.allocateCamera(type, nodeHandle, viewportDataInstance, preallocateHandle(handle));
    }

    NodeHandle SceneAllocateHelper::allocateNode(UInt32 childrenCount, NodeHandle handle)
    {
        return m_scene.allocateNode(childrenCount, preallocateHandle(handle));
    }

    TransformHandle SceneAllocateHelper::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        return m_scene.allocateTransform(nodeHandle, preallocateHandle(handle));
    }

    DataLayoutHandle SceneAllocateHelper::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        return m_scene.allocateDataLayout(dataFields, preallocateHandle(handle));
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

    RenderPassHandle SceneAllocateHelper::allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle handle)
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

    StreamTextureHandle SceneAllocateHelper::allocateStreamTexture(uint32_t streamSource, ResourceContentHash fallbackTextureHash, StreamTextureHandle handle)
    {
        return m_scene.allocateStreamTexture(streamSource, fallbackTextureHash, preallocateHandle(handle));
    }

    DataSlotHandle SceneAllocateHelper::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        return m_scene.allocateDataSlot(dataSlot, preallocateHandle(handle));
    }

    DataBufferHandle SceneAllocateHelper::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
    {
        return m_scene.allocateDataBuffer(dataBufferType, dataType, maximumSizeInBytes, preallocateHandle(handle));
    }

    TextureBufferHandle SceneAllocateHelper::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle /*= TextureBufferHandle::Invalid()*/)
    {
        return m_scene.allocateTextureBuffer(textureFormat, mipMapDimensions, preallocateHandle(handle));
    }
}
