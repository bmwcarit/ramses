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

namespace ramses_internal
{
    struct Renderable
    {
        ResourceContentHash effectResource;
        NodeHandle node;
        Bool isVisible = true;

        UInt32 startIndex = 0u;
        UInt32 indexCount = 0u;
        UInt32 instanceCount = 1u;

        DataInstanceHandle dataInstances[ERenderableDataSlotType_MAX_SLOTS];
        RenderStateHandle renderState;
    };
}

#endif
