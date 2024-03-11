//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/EDataType.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class EShaderStage
    {
        Vertex = 0,
        Fragment,
        Geometry,
    };

    static constexpr const char* const EShaderStageNames[] =
    {
        "Vertex",
        "Fragment",
        "Geometry",
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EShaderStage,
                                        "EShaderStage",
                                        ramses_internal::EShaderStageNames,
                                        ramses_internal::EShaderStage::Geometry);

