//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EVisibilityMode.h"
#include "internal/SceneGraph/SceneAPI/Renderable.h"
#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    const std::array VisibilityModeNames = {"Off", "Invisible", "Visible"};
    ENUM_TO_STRING(EVisibilityMode, VisibilityModeNames, EVisibilityMode::Visible);
}
