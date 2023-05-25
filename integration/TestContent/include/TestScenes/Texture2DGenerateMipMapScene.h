//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DGENERATEMIPMAPSCENE_H
#define RAMSES_TEXTURE2DGENERATEMIPMAPSCENE_H

#include "IntegrationScene.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"

namespace ramses
{
    class TextureSampler;
    class GeometryBinding;
    class ArrayResource;
}

namespace ramses_internal
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
        ramses::TextureSampler* createTexture2DSampler(UInt32 width = 2u, UInt32 height = 2u, UInt8 transparency = 0u);

        const ramses::ArrayResource* m_indexArray;
        const ramses::ArrayResource* m_vertexPositions;
        const ramses::ArrayResource* m_textureCoords;
    };
}

#endif
