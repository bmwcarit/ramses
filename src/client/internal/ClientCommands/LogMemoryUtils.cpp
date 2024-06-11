//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogMemoryUtils.h"
#include "impl/ResourceImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "ramses/client/Resource.h"
#include "internal/SceneGraph/SceneAPI/Camera.h"
#include "internal/SceneGraph/SceneAPI/Renderable.h"
#include "internal/SceneGraph/SceneAPI/RenderPass.h"
#include "internal/SceneGraph/SceneAPI/RenderGroup.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/TextureBuffer.h"
#include "internal/SceneGraph/SceneAPI/GeometryDataBuffer.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/SceneAPI/RenderTarget.h"
#include "internal/SceneGraph/SceneAPI/BlitPass.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/TopologyNode.h"
#include "internal/SceneGraph/Scene/TopologyTransform.h"
#include "internal/SceneGraph/Scene/DataLayout.h"
#include "internal/SceneGraph/Scene/DataInstance.h"

namespace ramses::internal
{
    MemoryInfoVector GetMemoryInfoFromScene(const SceneImpl& scene)
    {
        MemoryInfoVector memoryInfos;
        SceneObjectVector resources;
        scene.getObjectRegistry().getObjectsOfType(resources, ERamsesObjectType::Resource);

        for (const auto it : resources)
        {
            const Resource& resource = RamsesObjectTypeUtils::ConvertTo<Resource>(*it);
            const IResource* resourceObject  = scene.getClientImpl().getResource(resource.impl().getLowlevelResourceHash()).get();

            if (nullptr != resourceObject)
            {
                MemoryInfo info;
                info.memoryUsage = resourceObject->getCompressedDataSize() + resourceObject->getDecompressedDataSize();
                info.logInfoMesage = fmt::format("Resource \"{}\"\ttype {} mem compressed {} uncompressed {} ",
                                                 resource.getName(),
                                                 EnumToString(resourceObject->getTypeID()),
                                                 resourceObject->getCompressedDataSize(),
                                                 resourceObject->getDecompressedDataSize());
                memoryInfos.push_back(info);
            }
        }

        auto createMemInfo = [](const auto& logMessage, uint32_t numElements, const std::function< size_t(uint32_t) >& sizeOfIndividualElement){
            MemoryInfo info;
            info.logInfoMesage = fmt::format("{} {} allocated", numElements, logMessage);

            for(uint32_t i = 0; i < numElements; ++i)
            {
                info.memoryUsage += static_cast<uint32_t>(sizeOfIndividualElement(i));
            }
            return info;
        };

        const ClientScene& iscene = scene.getIScene();

        memoryInfos.push_back(createMemInfo("Cameras",         iscene.getCameraCount(),         [](uint32_t /*unused*/){return sizeof(Camera);}));
        memoryInfos.push_back(createMemInfo("Renderables",     iscene.getRenderableCount(),     [](uint32_t /*unused*/){return sizeof(Renderable);}));
        memoryInfos.push_back(createMemInfo("RenderStates",    iscene.getRenderStateCount(),    [](uint32_t /*unused*/){return sizeof(RenderState);}));
        memoryInfos.push_back(createMemInfo("Transforms",      iscene.getTransformCount(),      [](uint32_t /*unused*/){return sizeof(TopologyTransform);}));
        memoryInfos.push_back(createMemInfo("BlitPasses",      iscene.getBlitPassCount(),       [](uint32_t /*unused*/){return sizeof(BlitPass);}));
        memoryInfos.push_back(createMemInfo("RenderBuffers",   iscene.getRenderBufferCount(),   [](uint32_t /*unused*/){return sizeof(RenderBuffer);}));
        memoryInfos.push_back(createMemInfo("TextureSamplers", iscene.getTextureSamplerCount(), [](uint32_t /*unused*/){return sizeof(TextureSampler);}));
        memoryInfos.push_back(createMemInfo("DataSlots",       iscene.getDataSlotCount(),       [](uint32_t /*unused*/){return sizeof(DataSlot);}));

        memoryInfos.push_back(createMemInfo("Nodes",           iscene.getNodeCount(), [&iscene](uint32_t h){
            return sizeof(TopologyNode)          + (iscene.isNodeAllocated(NodeHandle(h))                   ? iscene.getChildCount(NodeHandle(h)) * sizeof(NodeHandle) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("RenderGroups",    iscene.getRenderGroupCount(), [&iscene](uint32_t h){
            return sizeof(RenderGroup)           + (iscene.isRenderGroupAllocated(RenderGroupHandle(h))     ? iscene.getRenderGroup(RenderGroupHandle(h)).renderables.size() * sizeof(RenderableOrderEntry) + iscene.getRenderGroup(RenderGroupHandle(h)).renderGroups.size() * sizeof(RenderGroupOrderEntry) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("RenderPasses",    iscene.getRenderPassCount(), [&iscene](uint32_t h){
            return sizeof(RenderPass)            + (iscene.isRenderPassAllocated(RenderPassHandle(h))       ? iscene.getRenderPass(RenderPassHandle(h)).renderGroups.size() * sizeof(RenderGroupOrderEntry) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("RenderTargets",   iscene.getRenderTargetCount(), [&iscene](uint32_t h){
            return sizeof(RenderTarget)          + (iscene.isRenderTargetAllocated(RenderTargetHandle(h))   ? iscene.getRenderTargetRenderBufferCount(RenderTargetHandle(h)) * sizeof(RenderBufferHandle) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("TextureBuffers",  iscene.getTextureBufferCount(), [&iscene](uint32_t h){
            return sizeof(TextureBuffer)         + (iscene.isTextureBufferAllocated(TextureBufferHandle(h)) ? TextureBuffer::GetMipMapDataSizeInBytes(iscene.getTextureBuffer(TextureBufferHandle(h))) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("DataBuffers",     iscene.getDataBufferCount(), [&iscene](uint32_t h){
            return sizeof(GeometryDataBuffer)    + (iscene.isDataBufferAllocated(DataBufferHandle(h))       ? iscene.getDataBuffer(DataBufferHandle(h)).data.size() : 0u);
        }));
        memoryInfos.push_back(createMemInfo("DataLayouts",     iscene.getDataLayoutCount(), [&iscene](uint32_t h){
            return sizeof(DataLayout)            + (iscene.isDataLayoutAllocated(DataLayoutHandle(h))       ? iscene.getDataLayout(DataLayoutHandle(h)).getFieldCount() * (sizeof(DataFieldInfo) + sizeof(uint32_t)) : 0u);
        }));
        memoryInfos.push_back(createMemInfo("DataInstances",   iscene.getDataInstanceCount(), [&iscene](uint32_t h){
            uint32_t memoryUsage = sizeof(DataInstance);
            if(iscene.isDataInstanceAllocated(DataInstanceHandle(h)))
            {
                const DataLayoutHandle layoutHandle = iscene.getLayoutOfDataInstance(DataInstanceHandle(h));
                const DataLayout& layout = iscene.getDataLayout(layoutHandle);
                memoryUsage += layout.getTotalSize();
            }
            return memoryUsage;
        }));
        return memoryInfos;
    }
}
