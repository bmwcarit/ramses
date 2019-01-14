//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ETOUCHEVENTTYPE_H
#define RAMSES_ETOUCHEVENTTYPE_H

#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum ETouchEventType
    {
        ETouchEventType_Invalid = 0,

        ETouchEventType_Down,
        ETouchEventType_Up,

        ETouchEventType_Move,

        ETouchEventType_NUMBER_OF_ELEMENTS
    };

    static const Char* TouchEventTypeNames[] =
    {
        "ETouchEventType_Invalid",
        "ETouchEventType_Down",
        "ETouchEventType_Up",
        "ETouchEventType_Move"
    };

    ENUM_TO_STRING(ETouchEventType, TouchEventTypeNames, ETouchEventType_NUMBER_OF_ELEMENTS);
}
#endif
