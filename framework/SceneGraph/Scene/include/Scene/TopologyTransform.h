//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TOPOLOGYTRANSFORM_H
#define RAMSES_TOPOLOGYTRANSFORM_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/ERotationType.h"
#include "SceneAPI/IScene.h"
#include "Math3d/Vector3.h"
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    struct TopologyTransform
    {
        Vector3 translation = IScene::IdentityTranslation;
        Vector4 rotation    = IScene::IdentityRotation;
        Vector3 scaling     = IScene::IdentityScaling;
        ERotationType rotationType = ERotationType::Euler_XYZ;

        NodeHandle node;
    };

    ASSERT_MOVABLE(TopologyTransform)
}

#endif
