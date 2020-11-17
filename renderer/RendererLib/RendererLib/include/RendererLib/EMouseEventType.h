//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMOUSEEVENTTYPE_H
#define RAMSES_EMOUSEEVENTTYPE_H

#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum EMouseEventType
    {
        EMouseEventType_Invalid          = 0,

        EMouseEventType_LeftButtonDown,
        EMouseEventType_LeftButtonUp,
        EMouseEventType_RightButtonDown,
        EMouseEventType_RightButtonUp,
        EMouseEventType_MiddleButtonDown,
        EMouseEventType_MiddleButtonUp,

        EMouseEventType_WheelUp,
        EMouseEventType_WheelDown,

        EMouseEventType_Move,

        EMouseEventType_WindowEnter,
        EMouseEventType_WindowLeave,

        EMouseEventType_NUMBER_OF_ELEMENTS
    };

    static const char* MouseEventTypeNames[] =
    {
        "EMouseEventType_Invalid",
        "EMouseEventType_LeftButtonDown",
        "EMouseEventType_LeftButtonUp",
        "EMouseEventType_RightButtonDown",
        "EMouseEventType_RightButtonUp",
        "EMouseEventType_MiddleButtonDown",
        "EMouseEventType_MiddleButtonUp",
        "EMouseEventType_WheelUp",
        "EMouseEventType_WheelDown",
        "EMouseEventType_Move",
        "EMouseEventType_WindowEnter",
        "EMouseEventType_WindowLeave"
    };

    ENUM_TO_STRING(EMouseEventType, MouseEventTypeNames, EMouseEventType_NUMBER_OF_ELEMENTS);
}
#endif
