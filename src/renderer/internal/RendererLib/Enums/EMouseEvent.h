//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"
#include "ramses/renderer/Types.h"

namespace ramses::internal
{
    using ramses::EMouseEvent;

    const std::array MouseEventNames =
    {
        "Invalid",
        "LeftButtonDown",
        "LeftButtonUp",
        "RightButtonDown",
        "RightButtonUp",
        "MiddleButtonDown",
        "MiddleButtonUp",
        "WheelUp",
        "WheelDown",
        "Move",
        "WindowEnter",
        "WindowLeave"
    };

    ENUM_TO_STRING(EMouseEvent, MouseEventNames, EMouseEvent::WindowLeave);
}

