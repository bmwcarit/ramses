//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"

#include <string>

namespace ramses
{
    class Effect;
    class Appearance;
    class Geometry;
}

namespace ramses::internal
{
    class CustomShaderTestScene : public IntegrationScene
    {
    public:
        CustomShaderTestScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            EXPLICIT_ATTRIBUTE_LOCATION = 0,
            EXPLICIT_ATTRIBUTE_LOCATION_SWAPPED
        };

    private:
        [[nodiscard]] static std::string GetEffectNameFromState(uint32_t state);
        void createGeometry();
        void initInputs();

        ramses::Effect&          m_effect;
        ramses::Appearance&      m_appearance;
        ramses::Geometry& m_geometryBinding;
    };
}

