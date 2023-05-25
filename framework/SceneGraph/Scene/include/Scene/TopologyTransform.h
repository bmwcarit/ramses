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
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    struct TopologyTransform
    {
        glm::vec3 translation = IScene::IdentityTranslation;
        glm::vec4 rotation    = IScene::IdentityRotation;
        glm::vec3 scaling     = IScene::IdentityScaling;
        ERotationType rotationType = ERotationType::Euler_XYZ;

        NodeHandle node;
    };

    ASSERT_MOVABLE(TopologyTransform)
}

#endif
