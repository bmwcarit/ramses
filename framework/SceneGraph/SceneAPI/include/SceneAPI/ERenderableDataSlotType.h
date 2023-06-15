//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_ERENDERABLEDATASLOTTYPE_H
#define RAMSES_SCENEAPI_ERENDERABLEDATASLOTTYPE_H

#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum ERenderableDataSlotType
    {
        ERenderableDataSlotType_Geometry = 0,
        ERenderableDataSlotType_Uniforms,

        ERenderableDataSlotType_MAX_SLOTS // must be last, used for checking
    };


    const std::array RenderableDataSlotTypeNames =
    {
        "Geometry",
        "Uniforms",
    };

    ENUM_TO_STRING(ERenderableDataSlotType, RenderableDataSlotTypeNames, ERenderableDataSlotType_MAX_SLOTS);

}

#endif
