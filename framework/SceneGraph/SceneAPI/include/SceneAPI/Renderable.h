//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERABLE_H
#define RAMSES_INTERNAL_RENDERABLE_H

#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ERenderableDataSlotType.h"

namespace ramses_internal
{
    enum class EVisibilityMode : int8_t
    {
        Off = 0,
        Invisible,
        Visible
    };

    struct Renderable
    {
        NodeHandle node;
        EVisibilityMode visibilityMode = EVisibilityMode::Visible;

        UInt32 startIndex = 0u;
        UInt32 indexCount = 0u;
        UInt32 instanceCount = 1u;
        UInt32 startVertex = 0u;

        DataInstanceHandle dataInstances[ERenderableDataSlotType_MAX_SLOTS];
        RenderStateHandle renderState;
    };
}

#endif
