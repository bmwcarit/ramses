//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CUSTOMSHADERTESTSCENE_H
#define RAMSES_CUSTOMSHADERTESTSCENE_H

#include "IntegrationScene.h"

namespace ramses
{
    class Effect;
    class Appearance;
    class GeometryBinding;
}

namespace ramses_internal
{
    class CustomShaderTestScene : public IntegrationScene
    {
    public:
        CustomShaderTestScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            EXPLICIT_ATTRIBUTE_LOCATION = 0,
            EXPLICIT_ATTRIBUTE_LOCATION_SWAPPED
        };

    private:
        String getEffectNameFromState(UInt32 state) const;
        void createGeometry();
        void initInputs();

        ramses::Effect&          m_effect;
        ramses::Appearance&      m_appearance;
        ramses::GeometryBinding& m_geometryBinding;
    };
}

#endif

