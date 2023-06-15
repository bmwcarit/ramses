//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GEOMETRYSHADERSCENE_H
#define RAMSES_GEOMETRYSHADERSCENE_H

#include "IntegrationScene.h"
#include "ramses-framework-api/AppearanceEnums.h"

namespace ramses_internal
{
    class GeometryShaderScene : public IntegrationScene
    {
    public:
        GeometryShaderScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT,
            GLSL320_POINTS_IN_LINE_STRIP_OUT,
            GLSL320_POINTS_IN_POINTS_OUT,
            GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT,
            GLSL320_TRIANGLES_IN_POINTS_OUT,
            GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT,
            GLSL310_POINTS_IN_LINE_STRIP_OUT,
            GLSL310_POINTS_IN_POINTS_OUT,
            GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT,
            GLSL310_TRIANGLES_IN_POINTS_OUT,
        };

    private:
        const ramses::Effect& createTestEffect(uint32_t state);
        std::string getGeometryShaderLayout(uint32_t state);
        std::string getShaderVersion(uint32_t state);
        std::string getGeometryShaderExtensions(uint32_t state);
        ramses::EDrawMode getDrawMode(uint32_t state);

        const ramses::Effect& m_effect;
    };
}

#endif

