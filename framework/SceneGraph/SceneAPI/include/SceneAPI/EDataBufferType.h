//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_EDATABUFFERTYPE_H
#define RAMSES_FRAMEWORK_EDATABUFFERTYPE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class EDataBufferType : UInt8
    {
        Invalid = 0,
        IndexBuffer,
        VertexBuffer,

        NUMBER_OF_ELEMENTS
    };

    const std::array DataBufferTypeNames =
    {
        "EDataBufferType::Invalid",
        "EDataBufferType::IndexBuffer",
        "EDataBufferType::VertexBuffer",
    };

    ENUM_TO_STRING(EDataBufferType, DataBufferTypeNames, EDataBufferType::NUMBER_OF_ELEMENTS);
}

#endif
