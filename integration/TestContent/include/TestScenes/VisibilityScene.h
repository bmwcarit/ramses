//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VISIBILITYSCENE_H
#define RAMSES_VISIBILITYSCENE_H

#include "IntegrationScene.h"

namespace ramses_internal
{
    class VisibilityScene : public IntegrationScene
    {
    public:
        VisibilityScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& position);

        void setState(UInt32 state);

        enum
        {
            VISIBILITY_OFF = 0,
            VISIBILITY_PARTIAL_ON,
            VISIBILITY_ON,
        };
    };
}

#endif
