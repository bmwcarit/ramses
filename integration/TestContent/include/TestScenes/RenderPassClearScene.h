//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSCLEARSCENE_H
#define RAMSES_RENDERPASSCLEARSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class RenderPassClearScene : public IntegrationScene
    {
    public:
        RenderPassClearScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

    private:
        void addTriangleMesh(ramses::Triangle& triangle, ramses::RenderGroup& targetGroup, float x, float y, float z);
        void renderBufferToScreen(ramses::RenderBuffer& renderBuffer);
        ramses::RenderTarget& createRenderTarget();
        const ramses::MeshNode& createQuadWithTexture(const ramses::RenderBuffer& renderBuffer);
        const ramses::PerspectiveCamera& createCamera();

        ramses::Effect& m_effect;

        ramses::Triangle m_blueTriangle;
        ramses::Triangle m_redTriangle;
        ramses::Triangle m_greenTriangle;

        ramses::RenderBuffer& m_colorBuffer;
        ramses::RenderBuffer& m_depthStencilBuffer;
    };
}

#endif //RAMSES_RENDERPASSCLEARSCENE_H

