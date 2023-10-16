//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/Core/Utils/AssertMovable.h"

#include <cstdint>

namespace ramses::internal
{
    struct TopologyNode
    {
        NodeHandleVector children;
        NodeHandle       parent;
    };

    ASSERT_MOVABLE(TopologyNode)
}
