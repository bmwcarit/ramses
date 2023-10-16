//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "Triangle.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"

namespace ramses::internal
{
    class RenderPassClearScene : public IntegrationScene
    {
    public:
        RenderPassClearScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

    private:
        void addTriangleMesh(Triangle& triangle, ramses::RenderGroup& targetGroup, float x, float y, float z);
        void renderBufferToScreen(ramses::RenderBuffer& renderBuffer);
        ramses::RenderTarget& createRenderTarget();
        const ramses::MeshNode& createQuadWithTexture(const ramses::RenderBuffer& renderBuffer);
        const ramses::PerspectiveCamera& createCamera();

        ramses::Effect& m_effect;

        Triangle m_blueTriangle;
        Triangle m_redTriangle;
        Triangle m_greenTriangle;

        ramses::RenderBuffer& m_colorBuffer;
        ramses::RenderBuffer& m_depthStencilBuffer;
    };
}
