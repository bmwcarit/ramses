//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EFeatureLevel.h"
#include "internal/Core/Utils/EnumTraits.h"

namespace ramses
{
    [[nodiscard]] inline constexpr bool IsFeatureLevel(std::underlying_type_t<EFeatureLevel> featureLevel)
    {
#if defined(_MSC_VER) && _MSC_VER < 1925
        // older versions of MSCV don't generate a proper __FUNCSIG__, if the enum contains duplicate symbols
        // for the same numeric value (EFeatureLevel_Latest)
#else
        static_assert (ramses::internal::EnumTraits::VerifyElementCountIfSupported<EFeatureLevel>(EFeatureLevel_Latest),
            "EFeatureLevel_Latest must refer to the latest feature level");
#endif
        return (featureLevel >= 1) && (featureLevel <= EFeatureLevel_Latest);
    }
}
