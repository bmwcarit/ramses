//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

#include <cstdint>

namespace ramses::internal
{
    enum class EDataSlotType
    {
        TransformationProvider = 0,
        TransformationConsumer,
        DataProvider,
        DataConsumer,
        TextureProvider,
        TextureConsumer,
        Undefined,
    };

    const std::array DataSlotTypeNames =
    {
        "TransformationProvider",
        "TransformationConsumer",
        "DataProvider",
        "DataConsumer",
        "TextureProvider",
        "TextureConsumer",
        "Undefined"
    };

    ENUM_TO_STRING(EDataSlotType, DataSlotTypeNames, EDataSlotType::Undefined);
}
