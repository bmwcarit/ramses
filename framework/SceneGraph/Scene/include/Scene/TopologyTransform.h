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
#include "Math3d/Vector3.h"

namespace ramses_internal
{
    struct TopologyTransform
    {
        Vector3 translation = { 0.f, 0.f, 0.f };
        Vector3 rotation    = { 0.f, 0.f, 0.f };
        Vector3 scaling     = { 1.f, 1.f, 1.f };

        NodeHandle node;
    };

    static_assert(std::is_nothrow_move_constructible<TopologyTransform>::value &&
        std::is_nothrow_move_assignable<TopologyTransform>::value, "Transform must be movable");
}

#endif
