//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "ramses/client/UniformInput.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/MipLevelData.h"

namespace ramses
{
    class TextureSampler;
    class Geometry;
    class ArrayResource;
}

namespace ramses::internal
{

    class Texture2DGenerateMipMapScene : public IntegrationScene
    {
    public:
        Texture2DGenerateMipMapScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum EState
        {
            EState_GenerateMipMapSingle = 0,
            EState_GenerateMipMapMultiple
        };

    protected:
        void createOrthoCamera();
        void createMesh(const ramses::TextureSampler& sampler, float translateXY = 0.0f, float scale = 1.0f);
        void createGeometry();
        ramses::TextureSampler* createTexture2DSampler(uint32_t width = 2u, uint32_t height = 2u, uint8_t transparency = 0u);

        const ramses::ArrayResource* m_indexArray = nullptr;
        const ramses::ArrayResource* m_vertexPositions = nullptr;
        const ramses::ArrayResource* m_textureCoords = nullptr;
    };
}
