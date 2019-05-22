//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DSAMPLINGSCENE_H
#define RAMSES_TEXTURE2DSAMPLINGSCENE_H

#include "IntegrationScene.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"

namespace ramses
{
    class TextureSampler;
    class GeometryBinding;
    class UInt16Array;
    class Vector3fArray;
    class Vector2fArray;
}

namespace ramses_internal
{

    class Texture2DSamplingScene : public IntegrationScene
    {
    public:
        Texture2DSamplingScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, uint32_t state, const Vector3& cameraPosition);

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

        ramses::ETextureSamplingMethod getMinSamplingMethod(EState state) const;
        ramses::ETextureSamplingMethod getMagSamplingMethod(EState state) const;

        const ramses::UInt16Array* m_indexArray;
        const ramses::Vector3fArray* m_vertexPositions;
        const ramses::Vector2fArray* m_textureCoords;
    };
}

#endif
