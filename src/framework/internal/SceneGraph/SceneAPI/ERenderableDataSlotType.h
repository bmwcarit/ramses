//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum ERenderableDataSlotType
    {
        ERenderableDataSlotType_Geometry = 0,
        ERenderableDataSlotType_Uniforms,
    };


    const std::array RenderableDataSlotTypeNames =
    {
        "Geometry",
        "Uniforms",
    };

    const size_t ERenderableDataSlotType_MAX_SLOTS = 2u;
    static_assert(EnumTraits::VerifyElementCountIfSupported<ERenderableDataSlotType>(ERenderableDataSlotType_MAX_SLOTS));

    ENUM_TO_STRING(ERenderableDataSlotType, RenderableDataSlotTypeNames, ERenderableDataSlotType_Uniforms);

}
