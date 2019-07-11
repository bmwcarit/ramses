//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogMemoryUtils.h"
#include "ResourceImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "ramses-client-api/Resource.h"
#include "SceneAPI/Camera.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/RenderPass.h"
#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/TextureBuffer.h"
#include "SceneAPI/GeometryDataBuffer.h"
#include "SceneAPI/StreamTexture.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/RenderTarget.h"
#include "SceneAPI/BlitPass.h"
#include "Scene/ClientScene.h"
#include "Scene/TopologyNode.h"
#include "Scene/TopologyTransform.h"
#include "Scene/DataLayout.h"
#include "Scene/DataInstance.h"

namespace ramses_internal
{
    MemoryInfoVector GetMemoryInfoFromScene(const ramses::SceneImpl& scene)
    {
        MemoryInfoVector memoryInfos;
        ramses::RamsesObjectVector resources = scene.getClientImpl().getListOfResourceObjects(ramses::ERamsesObjectType_Resource);

        for (const auto it : resources)
        {
            const ramses::Resource& resource = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*it);
            const IResource* resourceObject  = scene.getClientImpl().getResource(resource.impl.getLowlevelResourceHash()).getResourceObject();

            if (nullptr != resourceObject)
            {
                StringOutputStream stream;
                stream << "Resource \"" << resource.getName() << "\"\t";
                stream << "type " << EnumToString(resourceObject->getTypeID()) << " ";
                stream << "mem compressed " << resourceObject->getCompressedDataSize() << " ";
                stream << "uncompressed " << resourceObject->getDecompressedDataSize() << " ";

                MemoryInfo info;
                info.memoryUsage = resourceObject->getCompressedDataSize() + resourceObject->getDecompressedDataSize();
                info.logInfoMesage = stream.release();
                memoryInfos.push_back(info);
            }
        }

        auto createMemInfo = [](const String& logMessage, uint32_t numElements, std::function< size_t(uint32_t) >sizeOfIndividualElement){
            StringOutputStream stream;
            stream << numElements << " " << logMessage << " allocated";

            MemoryInfo info;
            info.logInfoMesage = stream.release();

            for(uint32_t i = 0; i < numElements; ++i)
            {
                info.memoryUsage += static_cast<uint32_t>(sizeOfIndividualElement(i));
            }
            return info;
        };

        const ClientScene& iscene = scene.getIScene();

        memoryInfos.push_back(createMemInfo("Cameras",         iscene.getCameraCount(),         [](uint32_t){return sizeof(Camera);}));
        memoryInfos.push_back(createMemInfo("Renderables",     iscene.getRenderableCount(),     [](uint32_t){return sizeof(Renderable);}));
        memoryInfos.push_back(createMemInfo("RenderStates",    iscene.getRenderStateCount(),    [](uint32_t){return sizeof(RenderState);}));
        memoryInfos.push_back(createMemInfo("Transforms",      iscene.getTransformCount(),      [](uint32_t){return sizeof(TopologyTransform);}));
        memoryInfos.push_back(createMemInfo("BlitPasses",      iscene.getBlitPassCount(),       [](uint32_t){return sizeof(BlitPass);}));
        memoryInfos.push_back(createMemInfo("RenderBuffers",   iscene.getRenderBufferCount(),   [](uint32_t){return sizeof(RenderBuffer);}));
        memoryInfos.push_back(createMemInfo("TextureSamplers", iscene.getTextureSamplerCount(), [](uint32_t){return sizeof(TextureSampler);}));
        memoryInfos.push_back(createMemInfo("StreamTextures",  iscene.getStreamTextureCount(),  [](uint32_t){return sizeof(StreamTexture);}));
        memoryInfos.push_back(createMemInfo("DataSlots",       iscene.getDataSlotCount(),       [](uint32_t){return sizeof(DataSlot);}));

        memoryInfos.push_back(createMemInfo("Nodes",           iscene.getNodeCount(),           [&iscene](uint32_t h){return sizeof(TopologyNode)          + (iscene.isNodeAllocated(NodeHandle(h))                   ? iscene.getChildCount(NodeHandle(h)) * sizeof(NodeHandle) : 0u);}));
        memoryInfos.push_back(createMemInfo("RenderGroups",    iscene.getRenderGroupCount(),    [&iscene](uint32_t h){return sizeof(RenderGroup)           + (iscene.isRenderGroupAllocated(RenderGroupHandle(h))     ? iscene.getRenderGroup(RenderGroupHandle(h)).renderables.size() * sizeof(RenderableOrderEntry) + iscene.getRenderGroup(RenderGroupHandle(h)).renderGroups.size() * sizeof(RenderGroupOrderEntry) : 0u);}));
        memoryInfos.push_back(createMemInfo("RenderPasses",    iscene.getRenderPassCount(),     [&iscene](uint32_t h){return sizeof(RenderPass)            + (iscene.isRenderPassAllocated(RenderPassHandle(h))       ? iscene.getRenderPass(RenderPassHandle(h)).renderGroups.size() * sizeof(RenderGroupOrderEntry) : 0u);}));
        memoryInfos.push_back(createMemInfo("RenderTargets",   iscene.getRenderTargetCount(),   [&iscene](uint32_t h){return sizeof(RenderTarget)          + (iscene.isRenderTargetAllocated(RenderTargetHandle(h))   ? iscene.getRenderTargetRenderBufferCount(RenderTargetHandle(h)) * sizeof(RenderBufferHandle) : 0u);}));
        memoryInfos.push_back(createMemInfo("TextureBuffers",  iscene.getTextureBufferCount(),  [&iscene](uint32_t h){return sizeof(TextureBuffer)         + (iscene.isTextureBufferAllocated(TextureBufferHandle(h)) ? TextureBuffer::GetMipMapDataSizeInBytes(iscene.getTextureBuffer(TextureBufferHandle(h))) : 0u);}));
        memoryInfos.push_back(createMemInfo("DataBuffers",     iscene.getDataBufferCount(),     [&iscene](uint32_t h){return sizeof(GeometryDataBuffer)    + (iscene.isDataBufferAllocated(DataBufferHandle(h))       ? iscene.getDataBuffer(DataBufferHandle(h)).data.size() : 0u);}));
        memoryInfos.push_back(createMemInfo("DataLayouts",     iscene.getDataLayoutCount(),     [&iscene](uint32_t h){return sizeof(DataLayout)            + (iscene.isDataLayoutAllocated(DataLayoutHandle(h))       ? iscene.getDataLayout(DataLayoutHandle(h)).getFieldCount() * (sizeof(DataFieldInfo) + sizeof(uint32_t)) : 0u);}));
        memoryInfos.push_back(createMemInfo("DataInstances",   iscene.getDataInstanceCount(),   [&iscene](uint32_t h){
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
