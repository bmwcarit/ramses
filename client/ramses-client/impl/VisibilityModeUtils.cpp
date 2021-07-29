//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "VisibilityModeUtils.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    ramses::EVisibilityMode VisibilityModeUtils::ConvertToHL(ramses_internal::EVisibilityMode mode)
    {
        switch (mode)
        {
        case ramses_internal::EVisibilityMode::Off:
            return EVisibilityMode::Off;
        case ramses_internal::EVisibilityMode::Invisible:
            return EVisibilityMode::Invisible;
        case ramses_internal::EVisibilityMode::Visible:
            return EVisibilityMode::Visible;
        }
        assert(!"unreachable code");
        return EVisibilityMode::Visible;
    }

    ramses_internal::EVisibilityMode VisibilityModeUtils::ConvertToLL(EVisibilityMode mode)
    {
        switch (mode)
        {
        case EVisibilityMode::Off:
            return ramses_internal::EVisibilityMode::Off;
        case EVisibilityMode::Invisible:
            return ramses_internal::EVisibilityMode::Invisible;
        case EVisibilityMode::Visible:
            return ramses_internal::EVisibilityMode::Visible;
        }
        assert(!"unreachable code");
        return ramses_internal::EVisibilityMode::Visible;
    }

    const char* const VisibilityModeNames[] =
    {
        "Off",
        "Invisible",
        "Visible"
    };
    ENUM_TO_STRING(EVisibilityMode, VisibilityModeNames, 3);

    const char* VisibilityModeUtils::ToString(EVisibilityMode mode)
    {
        return EnumToString(mode);
    }
}
