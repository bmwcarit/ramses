//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERESOURCEUPLOADER_H
#define RAMSES_SCENERESOURCEUPLOADER_H

#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class IScene;
    class IRendererResourceManager;

    class SceneResourceUploader
    {
    public:
        static void UploadRenderTarget(const IScene& scene, RenderTargetHandle renderTarget, IRendererResourceManager& resourceManager);
        static void UploadRenderBuffer(const IScene& scene, RenderBufferHandle renderBuffer, IRendererResourceManager& resourceManager);
        static void UploadBlitPassRenderTargets(const IScene& scene, BlitPassHandle blitPass, IRendererResourceManager& resourceManager);
        static void UploadTextureBuffer(const IScene& scene, TextureBufferHandle textureBuffer, IRendererResourceManager& resourceManager);
        static void UpdateTextureBuffer(const IScene& scene, TextureBufferHandle textureBuffer, IRendererResourceManager& resourceManager);
        static void UploadVertexArray(const IScene& scene, RenderableHandle renderableHandle, IRendererResourceManager& resourceManager);
    };
}

#endif
