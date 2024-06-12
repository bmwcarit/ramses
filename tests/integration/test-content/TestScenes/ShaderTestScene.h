//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "Triangle.h"

#include <string>

namespace ramses::internal
{
    class ShaderTestScene : public IntegrationScene
    {
    public:
        ShaderTestScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            DISCARD = 0,
            OPTIMIZED_INPUT,
            UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES,
            STRUCT_UNIFORM,
            TEXTURE_SIZE,
            BOOL_UNIFORM,
            UNIFORM_BUFFERS_STD140,
        };

    private:
        [[nodiscard]] static std::string GetEffectNameFromState(uint32_t state);
        void initInputs(uint32_t state);

        ramses::Effect&   m_effect;
        Triangle          m_triangle;
    };
}

