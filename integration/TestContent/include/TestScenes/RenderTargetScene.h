//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETSCENE_H
#define RAMSES_RENDERTARGETSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"

namespace ramses
{
    class RenderBuffer;
}

namespace ramses_internal
{
    class RenderTargetScene : public IntegrationScene
    {
    public:
        RenderTargetScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            PERSPECTIVE_PROJECTION = 0,
            ORTHOGRAPHIC_PROJECTION,

            RENDERBUFFER_FORMAT_RGBA4,
            RENDERBUFFER_FORMAT_R8,
            RENDERBUFFER_FORMAT_RG8,
            RENDERBUFFER_FORMAT_RGB8,
            RENDERBUFFER_FORMAT_R16F,
            RENDERBUFFER_FORMAT_R32F,
            RENDERBUFFER_FORMAT_RG16F,
            RENDERBUFFER_FORMAT_RG32F,
            RENDERBUFFER_FORMAT_RGB16F,
            RENDERBUFFER_FORMAT_RGB32F,
            RENDERBUFFER_FORMAT_RGBA16F,
            RENDERBUFFER_FORMAT_RGBA32F
        };

    private:
        const ramses::RenderBuffer& createRenderBuffer(uint32_t state);
        ramses::Camera* createCamera(uint32_t state);
        void initInputRenderPass(uint32_t state);
        void initFinalRenderPass();

        const ramses::RenderBuffer& m_renderBuffer;
    };
}

#endif
