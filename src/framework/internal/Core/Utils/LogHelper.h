//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogLevel.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace ramses::internal
{
    namespace LogHelper
    {
        using ContextFilter = std::pair<ELogLevel, std::string>;

        bool StringToLogLevel(std::string_view str, ELogLevel& logLevel);
        ELogLevel GetLoglevelFromInt(int32_t logLevelInt);

        std::vector<ContextFilter> ParseContextFilters(const std::string& filterCommand);
    }
}
