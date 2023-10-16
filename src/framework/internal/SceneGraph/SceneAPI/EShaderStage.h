//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/Core/Utils/LoggingUtils.h"

#include <array>

namespace ramses::internal
{
    enum class EShaderStage
    {
        Vertex = 0,
        Fragment,
        Geometry,
    };

    const std::array EShaderStageNames =
    {
        "Vertex",
        "Fragment",
        "Geometry",
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EShaderStage,
                                        "EShaderStage",
                                        ramses::internal::EShaderStageNames,
                                        ramses::internal::EShaderStage::Geometry);

