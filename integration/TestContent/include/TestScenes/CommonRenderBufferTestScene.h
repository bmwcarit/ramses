//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMONRENDERBUFFERTESTSCENE_H
#define RAMSES_COMMONRENDERBUFFERTESTSCENE_H

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

namespace ramses_internal
{
    class CommonRenderBufferTestScene : public IntegrationScene
    {
    public:
        CommonRenderBufferTestScene(ramses::Scene& scene, const Vector3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

    protected:
        const ramses::Effect&       getEffectRenderOneBuffer();
        const ramses::Effect&       getEffectRenderTwoBuffers();
        ramses::PerspectiveCamera&  createCamera(Float nearPlane = 1.0f, Float farPlane = 100.0f);
        const ramses::MeshNode&     createQuadWithTexture(const ramses::RenderBuffer& renderBuffer);
        ramses::MeshNode&           createMesh(const ramses::Effect& effect, ramses::TriangleAppearance::EColor color = ramses::TriangleAppearance::EColor_Red);
        ramses::RenderPass*         addRenderPassUsingRenderBufferAsQuadTexture(const ramses::MeshNode& quad);

    };
}

#endif
