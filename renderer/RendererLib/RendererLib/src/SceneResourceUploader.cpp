//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneResourceUploader.h"
#include "RendererLib/IRendererResourceManager.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/TextureBuffer.h"
#include "SceneAPI/BlitPass.h"
#include "Scene/DataLayout.h"

namespace ramses_internal
{
    void SceneResourceUploader::UploadRenderTarget(const IScene& scene, RenderTargetHandle renderTarget, IRendererResourceManager& resourceManager)
    {
        assert(scene.isRenderTargetAllocated(renderTarget));

        const UInt32 bufferCount = scene.getRenderTargetRenderBufferCount(renderTarget);
        RenderBufferHandleVector renderBufferHandles;
        renderBufferHandles.reserve(bufferCount);
        for (UInt32 bufferIdx = 0u; bufferIdx < bufferCount; ++bufferIdx)
        {
            const RenderBufferHandle buffer = scene.getRenderTargetRenderBuffer(renderTarget, bufferIdx);
            renderBufferHandles.push_back(buffer);
        }

        resourceManager.uploadRenderTarget(renderTarget, renderBufferHandles, scene.getSceneId());
    }

    void SceneResourceUploader::UploadRenderBuffer(const IScene& scene, RenderBufferHandle renderBuffer, IRendererResourceManager& resourceManager)
    {
        assert(scene.isRenderBufferAllocated(renderBuffer));
        resourceManager.uploadRenderTargetBuffer(renderBuffer, scene.getSceneId(), scene.getRenderBuffer(renderBuffer));
    }

    void SceneResourceUploader::UploadBlitPassRenderTargets(const IScene& scene, BlitPassHandle blitPass, IRendererResourceManager& resourceManager)
    {
        assert(scene.isBlitPassAllocated(blitPass));
        const BlitPass& blitPassData = scene.getBlitPass(blitPass);
        resourceManager.uploadBlitPassRenderTargets(blitPass, blitPassData.sourceRenderBuffer, blitPassData.destinationRenderBuffer, scene.getSceneId());
    }

    void SceneResourceUploader::UploadTextureBuffer(const IScene& scene, TextureBufferHandle textureBuffer, IRendererResourceManager& resourceManager)
    {
        const TextureBuffer& texBuffer = scene.getTextureBuffer(textureBuffer);
        const auto& mipMaps = texBuffer.mipMaps;
        const auto& mip0 = mipMaps[0];
        resourceManager.uploadTextureBuffer(textureBuffer, mip0.width, mip0.height, texBuffer.textureFormat, static_cast<UInt32>(mipMaps.size()), scene.getSceneId());
    }

    void SceneResourceUploader::UpdateTextureBuffer(const IScene& scene, TextureBufferHandle textureBuffer, IRendererResourceManager& resourceManager)
    {
        const TextureBuffer& texBuffer = scene.getTextureBuffer(textureBuffer);
        const auto& mipMaps = texBuffer.mipMaps;
        for (UInt32 mipLevel = 0u; mipLevel < static_cast<uint32_t>(mipMaps.size()); ++mipLevel)
        {
            const auto& mip = mipMaps[mipLevel];
            resourceManager.updateTextureBuffer(textureBuffer, mipLevel, 0u, 0u, mip.width, mip.height, texBuffer.mipMaps[mipLevel].data.data(), scene.getSceneId());
        }
    }

    void SceneResourceUploader::UploadVertexArray(const IScene& scene, RenderableHandle renderableHandle, IRendererResourceManager& resourceManager)
    {
        const auto& renderable = scene.getRenderable(renderableHandle);
        const auto geometryInstance = renderable.dataInstances[ERenderableDataSlotType_Geometry];
        const auto& geometryLayout = scene.getDataLayout(scene.getLayoutOfDataInstance(geometryInstance));
        const auto& effectHash = geometryLayout.getEffectHash();

        VertexArrayInfo vertexArrayInfo;
        vertexArrayInfo.shader = resourceManager.getResourceDeviceHandle(effectHash);

        const UInt attributesCount = geometryLayout.getFieldCount();
        for (DataFieldHandle attributeField(0u); attributeField < attributesCount; ++attributeField)
        {
            const auto& dataField = geometryLayout.getField(attributeField);
            const auto& dataResource = scene.getDataResource(geometryInstance, attributeField);

            DeviceResourceHandle bufferHandle;
            if (dataResource.dataBuffer.isValid())
                bufferHandle = resourceManager.getDataBufferDeviceHandle(dataResource.dataBuffer, scene.getSceneId());
            else if (dataResource.hash.isValid())
                bufferHandle = resourceManager.getResourceDeviceHandle(dataResource.hash);

            //all buffers must have valid device handles, except for index buffer since it's possible to have vertex array
            //without index buffer
            assert(bufferHandle.isValid() || attributeField == 0u);

            //indices are always in the 1st field (field Zero)
            if (attributeField == 0u)
                vertexArrayInfo.indexBuffer = bufferHandle;
            else
                vertexArrayInfo.vertexBuffers.push_back({ bufferHandle, attributeField - 1, dataResource.instancingDivisor, renderable.startVertex, dataField.dataType, dataResource.offsetWithinElementInBytes, dataResource.stride });
        }

        resourceManager.uploadVertexArray(renderableHandle, vertexArrayInfo, scene.getSceneId());
    }
}
