//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERGROUP_H
#define RAMSES_INTERNAL_RENDERGROUP_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneTypes.h"

namespace ramses_internal
{
    struct RenderableOrderEntry
    {
        RenderableHandle renderable;
        Int32            order;
    };
    typedef std::vector<RenderableOrderEntry> RenderableOrderVector;

    struct RenderGroup
    {
        RenderableOrderVector  renderables;
        RenderGroupOrderVector renderGroups;
    };

    static_assert(std::is_nothrow_move_constructible<RenderGroup>::value &&
        std::is_nothrow_move_assignable<RenderGroup>::value, "RenderGroup must be movable");
}

#endif
