//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANTIALIASINGSCENE_H
#define RAMSES_ANTIALIASINGSCENE_H

#include "IntegrationScene.h"

namespace ramses_internal
{
    class AntiAliasingScene : public IntegrationScene
    {
    public:
        AntiAliasingScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            NoAA_STATE = 0,
            MSAA_2_STATE,
            MSAA_4_STATE
        };
    };
}

#endif

