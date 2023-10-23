//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "TriangleAppearance.h"

namespace ramses
{
    class RenderBuffer;
    class RenderTarget;
    class PerspectiveCamera;
    class Effect;
    class MeshNode;
}

namespace ramses::internal
{
    class CommonRenderBufferTestScene : public IntegrationScene
    {
    public:
        CommonRenderBufferTestScene(ramses::Scene& scene, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

    protected:
        const ramses::Effect&       getEffectRenderOneBuffer();
        const ramses::Effect&       getEffectRenderTwoBuffers();
        ramses::PerspectiveCamera&  createCamera(float nearPlane = 1.0f, float farPlane = 100.0f);
        const ramses::MeshNode&     createQuadWithTexture(const ramses::RenderBuffer& renderBuffer);
        ramses::MeshNode&           createMesh(const ramses::Effect& effect, TriangleAppearance::EColor color = TriangleAppearance::EColor_Red);
        ramses::RenderPass*         addRenderPassUsingRenderBufferAsQuadTexture(const ramses::MeshNode& quad);

    };
}
