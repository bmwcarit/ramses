//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/FlushTimeInformation.h"

#include <cstdint>

namespace ramses::internal
{
    class EffectUniformTime
    {
    public:
        static int32_t GetMilliseconds(FlushTime::Clock::time_point epochBeginning);
    };
}

