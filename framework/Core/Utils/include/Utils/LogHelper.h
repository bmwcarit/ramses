//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_LOGHELPER_H
#define RAMSES_UTILS_LOGHELPER_H

#include "Utils/LogLevel.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/String.h"
#include "Collections/Pair.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    namespace LogHelper
    {
        typedef std::pair<ELogLevel, String> ContextFilter;

        Bool StringToLogLevel(String str, ELogLevel& logLevel);
        ELogLevel GetLoglevelFromInt(Int32 logLevelInt);

        std::vector<ContextFilter> ParseContextFilters(const String& filterCommand);
    }
}

#endif
