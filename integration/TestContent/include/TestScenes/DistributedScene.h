//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISTRIBUTEDSCENE_H
#define RAMSES_DISTRIBUTEDSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"

namespace ramses_internal
{
    class DistributedScene : public IntegrationScene
    {
    public:
        DistributedScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            SHOW_SCENE = 0,
            UNPUBLISH,
            DESTROY,
            DISCONNECT
        };

    private:
        void createContent();
    };
}

#endif
