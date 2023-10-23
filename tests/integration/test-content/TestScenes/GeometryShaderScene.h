//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "ramses/framework/AppearanceEnums.h"

namespace ramses::internal
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
        static std::string GetGeometryShaderLayout(uint32_t state);
        static std::string GetShaderVersion(uint32_t state);
        static std::string GetGeometryShaderExtensions(uint32_t state);
        static ramses::EDrawMode GetDrawMode(uint32_t state);

        const ramses::Effect& m_effect;
    };
}

