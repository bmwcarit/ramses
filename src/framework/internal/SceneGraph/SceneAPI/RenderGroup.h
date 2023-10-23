//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/Core/Utils/AssertMovable.h"

namespace ramses::internal
{
    struct RenderableOrderEntry
    {
        RenderableHandle renderable;
        int32_t          order{0};
    };
    using RenderableOrderVector = std::vector<RenderableOrderEntry>;

    struct RenderGroup
    {
        RenderableOrderVector  renderables;
        RenderGroupOrderVector renderGroups;
    };

    ASSERT_MOVABLE(RenderGroup)
}
