//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERTESTSCENE_H
#define RAMSES_SHADERTESTSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"

#include <string>

namespace ramses_internal
{
    class ShaderTestScene : public IntegrationScene
    {
    public:
        ShaderTestScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition);

        enum
        {
            DISCARD = 0,
            OPTIMIZED_INPUT,
            UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES,
            STRUCT_UNIFORM,
            TEXTURE_SIZE,
            BOOL_UNIFORM,
        };

    private:
        [[nodiscard]] std::string getEffectNameFromState(UInt32 state) const;
        void initInputs(UInt32 state);

        ramses::Effect&   m_effect;
        ramses::Triangle  m_triangle;
    };
}

#endif

