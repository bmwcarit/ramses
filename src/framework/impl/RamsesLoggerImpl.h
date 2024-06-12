//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include <string_view>

namespace ramses::internal
{
    class RamsesLogger;

    RAMSES_IMPL_EXPORT RamsesLogger& GetRamsesLogger();

    // These are only helpers to solve thread local storage across shared libraries for logging prefixes and not meant to be used,
    // use RamsesLoggerPrefixes instead.
    class RamsesLoggerPrefixesExported
    {
    private:
        RAMSES_IMPL_EXPORT static void SetRamsesLoggerPrefixesExported(std::string_view instance, std::string_view thread, std::string_view additional = {});
        RAMSES_IMPL_EXPORT static void SetRamsesLoggerPrefixAdditionalExported(std::string_view additional);

        friend class RamsesLoggerPrefixes;
    };
}
