//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum class EShaderWarningCategory
    {
        Unknown,
        UnusedVarying,
        UnusedUniform,
        UnusedVariable,
        InterfaceMismatch,
        PrecisionMismatch,
    };

    const std::array EShaderWarningCategoryNames = {
        "Unknown",
        "UnusedVarying",
        "UnusedUniform",
        "UnusedVariable",
        "InterfaceMismatch",
        "PrecisionMismatch",
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EShaderWarningCategory,
                                        "EShaderWarningCategory",
                                        ramses::internal::EShaderWarningCategoryNames,
                                        ramses::internal::EShaderWarningCategory::PrecisionMismatch);

