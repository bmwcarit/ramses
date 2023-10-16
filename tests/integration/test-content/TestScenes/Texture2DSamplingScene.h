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

    class Texture2DSamplingScene : public IntegrationScene
    {
    public:
        Texture2DSamplingScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum EState
        {
            EState_Nearest = 0,
            EState_NearestWithMipMaps,
            EState_Bilinear,
            EState_BilinearWithMipMaps,
            EState_Trilinear,
            EState_MinLinearMagNearest,
            EState_MinNearestMagLinear
        };

    protected:
        void createOrthoCamera();
        void createMesh(const ramses::TextureSampler& sampler, EState state);
        void createGeometry(EState state);
        void createGeometryMinLinearMagNearest();
        void createGeometryMinNearestMagLinear();
        void createGeometryNearest();
        void createGeometryBilinear();
        void createGeometryNearestWithMipMaps();
        void createGeometryBilinearWithMipMaps();
        void createGeometryTrilinear();

        void createTwoQuads(float x);

        [[nodiscard]] static ramses::ETextureSamplingMethod GetMinSamplingMethod(EState state);
        [[nodiscard]] static ramses::ETextureSamplingMethod GetMagSamplingMethod(EState state);

        const ramses::ArrayResource* m_indexArray = nullptr;
        const ramses::ArrayResource* m_vertexPositions = nullptr;
        const ramses::ArrayResource* m_textureCoords = nullptr;
    };
}
