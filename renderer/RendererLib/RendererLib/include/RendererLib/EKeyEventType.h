//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EKEYEVENTTYPE_H
#define RAMSES_EKEYEVENTTYPE_H

#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum EKeyEventType
    {
        EKeyEventType_Invalid = 0,

        EKeyEventType_Pressed,
        EKeyEventType_Released,

        EKeyEventType_NUMBER_OF_ELEMENTS
    };

    static const Char* KeyEventTypeNames[] =
    {
        "EKeyEventType_Invalid",
        "EKeyEventType_Pressed",
        "EKeyEventType_Released"
    };

    ENUM_TO_STRING(EKeyEventType, KeyEventTypeNames, EKeyEventType_NUMBER_OF_ELEMENTS);
}
#endif
