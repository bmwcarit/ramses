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

    void SceneResourceUploader::UploadTextureBuffer(const IScene& scene, TextureBufferHandle handle, IRendererResourceManager& resourceManager)
    {
        const TextureBuffer& texBuffer = scene.getTextureBuffer(handle);
        const MipMapSize mipMap0Size = texBuffer.mipMapDimensions.front();
        resourceManager.uploadTextureBuffer(handle, mipMap0Size.width, mipMap0Size.height, texBuffer.textureFormat, static_cast<UInt32>(texBuffer.mipMapDimensions.size()), scene.getSceneId());
    }

    void SceneResourceUploader::UpdateTextureBuffer(const IScene& scene, TextureBufferHandle handle, IRendererResourceManager& resourceManager)
    {
        const TextureBuffer& texBuffer = scene.getTextureBuffer(handle);
        const auto& mipSizes = texBuffer.mipMapDimensions;
        for (UInt32 mipLevel = 0u; mipLevel < mipSizes.size(); ++mipLevel)
        {
            const MipMapSize mipSize = mipSizes[mipLevel];
            resourceManager.updateTextureBuffer(handle, mipLevel, 0u, 0u, mipSize.width, mipSize.height, texBuffer.mipMapData[mipLevel].data(), scene.getSceneId());
        }
    }
}
