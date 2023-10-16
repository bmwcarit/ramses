//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/ERenderableDataSlotType.h"
#include "ramses/framework/EVisibilityMode.h"
#include <array>

namespace ramses::internal
{
    using ramses::EVisibilityMode;

    struct Renderable
    {
        NodeHandle node;
        EVisibilityMode visibilityMode = EVisibilityMode::Visible;

        uint32_t startIndex = 0u;
        uint32_t indexCount = 0u;
        uint32_t instanceCount = 1u;
        uint32_t startVertex = 0u;

        std::array<DataInstanceHandle, ERenderableDataSlotType_MAX_SLOTS> dataInstances;
        RenderStateHandle renderState;
    };
}
