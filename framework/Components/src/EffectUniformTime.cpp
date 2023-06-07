//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/EffectUniformTime.h"

namespace ramses_internal
{
    int32_t EffectUniformTime::GetMilliseconds(FlushTime::Clock::time_point epochBeginning)
    {
        constexpr int32_t limit = std::numeric_limits<int32_t>::max(); // wrap after ~24 days
        const auto nowMs        = std::chrono::duration_cast<std::chrono::milliseconds>(FlushTime::Clock::now() - epochBeginning);
        const auto nowMsWrapped = nowMs.count() % limit;
        return static_cast<int32_t>(nowMsWrapped);
    }
}
