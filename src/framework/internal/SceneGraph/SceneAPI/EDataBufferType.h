//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
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
    enum class EDataBufferType : uint8_t
    {
        Invalid = 0,
        IndexBuffer,
        VertexBuffer,
    };

    const std::array DataBufferTypeNames =
    {
        "EDataBufferType::Invalid",
        "EDataBufferType::IndexBuffer",
        "EDataBufferType::VertexBuffer",
    };

    ENUM_TO_STRING(EDataBufferType, DataBufferTypeNames, EDataBufferType::VertexBuffer);
}
