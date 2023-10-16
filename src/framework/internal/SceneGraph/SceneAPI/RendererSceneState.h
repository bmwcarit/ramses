//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"
#include "ramses/framework/RendererSceneState.h"
#include <cstdint>

namespace ramses::internal
{
    using ramses::RendererSceneState;

    const std::array RendererSceneStateNames =
    {
        "Unavailable",
        "Available",
        "Ready",
        "Rendered",
    };
    ENUM_TO_STRING(RendererSceneState, RendererSceneStateNames, RendererSceneState::Rendered);
}
