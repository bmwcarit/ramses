//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_TOPOLOGYNODE_H
#define RAMSES_INTERNAL_TOPOLOGYNODE_H

#include "SceneAPI/SceneTypes.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    struct TopologyNode
    {
        NodeHandleVector children;
        NodeHandle       parent;
    };

    ASSERT_MOVABLE(TopologyNode)
}

#endif
