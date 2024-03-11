//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Utils/LoggingUtils.h"

namespace ramses_internal
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

    static constexpr const char* const EShaderWarningCategoryNames[] = {
        "Unknown",
        "UnusedVarying",
        "UnusedUniform",
        "UnusedVariable",
        "InterfaceMismatch",
        "PrecisionMismatch",
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EShaderWarningCategory,
                                        "EShaderWarningCategory",
                                        ramses_internal::EShaderWarningCategoryNames,
                                        ramses_internal::EShaderWarningCategory::PrecisionMismatch);

