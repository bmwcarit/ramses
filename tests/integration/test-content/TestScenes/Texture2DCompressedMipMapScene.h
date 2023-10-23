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

    class Texture2DCompressedMipMapScene : public IntegrationScene
    {
    public:
        Texture2DCompressedMipMapScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum EState
        {
            EState_CompressedMipMap = 0
        };

    private:
        void createOrthoCamera();
        void createMesh(const ramses::TextureSampler& sampler);
        void createGeometry();

        const ramses::ArrayResource* m_indexArray = nullptr;
        const ramses::ArrayResource* m_vertexPositions = nullptr;
        const ramses::ArrayResource* m_textureCoords = nullptr;

        static constexpr uint32_t NumMipMaps = 2u;
        uint32_t m_textureWidth = 8;
        uint32_t m_textureHeight = 8;
    };
}
